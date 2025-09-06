#include "sample_search.hpp"

#include <cmath>

#include "utils/rng.hpp"

namespace {
// Checks if a non-empty vector of size <= 2 contains an element.
bool contains(const std::vector<Vertex_id_t>& ids, const Vertex_id_t id) {
    return ids[0] == id || (ids.size() == 2 && ids[1] == id);
}
}  // namespace

void SampledVertexTraversal::dfs(
    const VertexPtr& start, const SampledVertexTraversal& other_traversal) {
    std::vector stack = {start};
    visited[start->id] = true;
    while (!stack.empty()) {
        const auto current = stack.back();
        stack.pop_back();
        if (other_traversal.visited[current->id]) {
            context->new_scc->emplace_back(current);
            context->new_scc->emplace_back(root);
        }
        if (context->reached_counter[current->id] == 0)
            context->reached->emplace_back(current);
        context->reached_counter[current->id]++;
        for (const auto& neighbour : context->graph.get_neighbours(current)) {
            if (!visited[neighbour->id]) {
                visited[neighbour->id] = true;
                stack.emplace_back(neighbour);
            }
        }
    }
}

void SampledVertexTraversal::search_for_new_reachable(
    const VertexPtr& u, const VertexPtr& v,
    const SampledVertexTraversal& other_traversal) {
    if (visited[u->id] && !visited[v->id])
        dfs(v, other_traversal);
}

void SampledVertex::update_reachable(const VertexPtr& u, const VertexPtr& v) {
    forward.search_for_new_reachable(u, v, backward);
    backward.search_for_new_reachable(v, u, forward);
}

// We use Theorem 27 and sample vertices independently.
Sample::Sample(Graph& graph, std::optional<size_t> original_no_vertices,
               const SampleTraversalContextPtr& forward_context,
               const SampleTraversalContextPtr& backward_context) {
    auto no_vertices = graph.get_no_vertices();
    if (original_no_vertices.has_value())
        no_vertices = *original_no_vertices;
    std::set<size_t> picked_ids;
    while (picked_ids.size() < std::min(no_vertices, SAMPLE_SIZE)) {
        picked_ids.insert(
            RNG::instance().randint(0, static_cast<int>(no_vertices) - 1));
    }

    for (auto& id : picked_ids)
        sampled_vertices.emplace_back(graph.get_vertex_by_id(id),
                                      forward_context, backward_context);
}

size_t Sample::size() const { return sampled_vertices.size(); }

void Sample::process_edge(const VertexPtr& u, const VertexPtr& v) {
    for (auto& sampled : sampled_vertices)
        sampled.update_reachable(u, v);
}

void PartitionHeaps::clear_modified_heaps() { modified_heaps.clear(); }

void PartitionHeaps::create_empty_set(const Partition& partition) {
    heaps.emplace(partition, std::set<Vertex_id_t, DynamicOrderComparator>(
                                 order_comparator));
}

void PartitionHeaps::insert(const Partition& partition, const VertexPtr& u) {
    if (heaps.at(partition).empty())
        modified_heaps.emplace_back(partition);
    heaps.at(partition).insert(u->id);
}

void PartitionHeaps::process_heap(const DynamicOrderPtr& order,
                                  const Partition& partition,
                                  Vertex_id_t dummy_id) {
    while (!heaps.at(partition).empty()) {
        const auto vertex_iter = get_best_option(partition);
        const auto vertex_id = *vertex_iter;
        heaps.at(partition).erase(vertex_iter);
        order->remove(vertex_id);
        update_order(order, vertex_id, dummy_id);
    }
}

PartitionHeaps::Heap_iterator PartitionHeapsUp::get_best_option(
    const Partition& partition) {
    return --heaps.at(partition).end();
}

PartitionHeaps::Heap_iterator PartitionHeapsDown::get_best_option(
    const Partition& partition) {
    return heaps.at(partition).begin();
}

void PartitionHeapsUp::update_order(const DynamicOrderPtr& order,
                                    Vertex_id_t vertex_id,
                                    Vertex_id_t dummy_id) {
    order->insert_after(vertex_id, dummy_id);
}

void PartitionHeapsDown::update_order(const DynamicOrderPtr& order,
                                      Vertex_id_t vertex_id,
                                      Vertex_id_t dummy_id) {
    order->insert_before(vertex_id, dummy_id);
}

std::pair<Vertex_id_t, Vertex_id_t> PartitionsHandler::get_dummy_ids(
    const Partition& partition) const {
    return {dummy_ids.at(partition), dummy_ids.at(partition) + 1};
}

std::optional<Partition> PartitionsHandler::next_dummy(
    const Partition& partition) {
    const auto iter = dummy_ids.upper_bound(partition);
    if (iter == dummy_ids.end())
        return std::nullopt;
    return iter->first;
}

void PartitionsHandler::insert_dummy(const Partition& partition) {
    if (dummy_ids.contains(partition))
        return;

    const auto dummy_id_front = order->total_elements_capacity();
    const auto dummy_id_back = dummy_id_front + 1;
    dummy_ids.emplace(partition, dummy_id_front);
    order->extend_elements_capacity();
    order->extend_elements_capacity();

    up.create_empty_set(partition);
    down.create_empty_set(partition);

    const auto next_partition = next_dummy(partition);
    if (!next_partition.has_value())
        order->insert_back(dummy_id_front);
    else
        order->insert_before(dummy_id_front,
                             get_dummy_ids(*next_partition).first);

    order->insert_after(dummy_id_back, dummy_id_front);
}

void PartitionsHandler::fill_up_and_down(
    const SampleTraversalContextPtr& forward_context,
    const SampleTraversalContextPtr& backward_context) {
    for (const auto& u : *forward_context->reached) {
        const auto new_ancestors_no = forward_context->reached_counter[u->id];
        const auto new_descendants_no =
            backward_context->reached_counter[u->id];
        if (new_ancestors_no == 0 && new_descendants_no == 0)
            continue;
        forward_context->reached_counter[u->id] = 0;
        backward_context->reached_counter[u->id] = 0;

        const auto old_partition = partitions[u->id];
        Partition new_partition = {
            old_partition.ancestors_no + new_ancestors_no,
            old_partition.descendants_no + new_descendants_no};
        partitions[u->id] = new_partition;
        insert_dummy(new_partition);

        if (new_partition < old_partition)
            down.insert(new_partition, u);
        else
            up.insert(new_partition, u);
    }
}

void PartitionsHandler::process_up_and_down() {
    for (const auto& partition : up.get_modified_heaps())
        up.process_heap(order, partition, get_dummy_ids(partition).first);
    up.clear_modified_heaps();

    for (const auto& partition : down.get_modified_heaps())
        down.process_heap(order, partition, get_dummy_ids(partition).second);
    down.clear_modified_heaps();
}

void SampleSearch::remove_non_canonical_reached_from_sampled() {
    for (size_t i = 0; i < forward_context->reached->size();) {
        const auto u = forward_context->reached->at(i);
        if (find_representative_vertex(u) != u) {
            std::swap(forward_context->reached->at(i),
                      forward_context->reached->back());
            forward_context->reached->pop_back();
        } else {
            i++;
        }
    }
}

void SampleSearch::merge_scc_containing_sampled() {
    const auto new_scc = forward_context->new_scc;
    for (size_t i = 1; i < new_scc->size(); i++) {
        const auto union_result = find_union.union_elements(
            new_scc->at(i - 1)->id, new_scc->at(i)->id);
        if (union_result.has_value()) {
            const auto [_, old_repr_id] = *union_result;
            order->remove(old_repr_id);
        }
    }
}

std::vector<size_t> Explorer::status;
size_t Explorer::no_explorations;

bool Explorer::alive_or_dead(Vertex_id_t vertex_id) const {
    return alive.contains(vertex_id) || dead.contains(vertex_id);
}

std::optional<Vertex_id_t> Explorer::get_minimum_from_heap(
    const Vertex_heap& heap) {
    if (heap.empty())
        return std::nullopt;
    return *heap.begin();
}

std::optional<Vertex_id_t> Explorer::get_maximum_from_heap(
    const Vertex_heap& heap) {
    if (heap.empty())
        return std::nullopt;
    return *(--heap.end());
}

std::vector<Vertex_id_t> Explorer::get_dead_as_vector() {
    std::vector<Vertex_id_t> dead_vec = {dead.begin(), dead.end()};
    dead.clear();
    return dead_vec;
}

void Explorer::populate_dead_with_vector(
    const std::vector<Vertex_id_t>& dead_vec) {
    for (const auto& u_id : dead_vec)
        dead.insert(find_union.find_representant(u_id));
}

void Explorer::erase_from_dead(Vertex_id_t vertex_id) {
    const auto iter = dead.find(find_union.find_representant(vertex_id));
    if (iter != dead.end())
        dead.erase(iter);
}

bool Explorer::any_alive() {
    if (alive.empty())
        finished_processing_alive = true;
    return !alive.empty();
}

void Explorer::add_alive(Vertex_id_t vertex_id) {
    visited_scc[find_union.find_representant(vertex_id)] = no_explorations;
    alive.insert(vertex_id);
}

std::optional<Vertex_id_t> Explorer::get_maximum_dead() const {
    return get_maximum_from_heap(dead);
}

std::optional<Vertex_id_t> Explorer::get_minimum_dead() const {
    return get_minimum_from_heap(dead);
}

void Explorer::explore(Vertex_id_t vertex_id, const Explorer& other_explorer) {
    const auto vertex = graph.get_vertex_by_id(vertex_id);
    const auto repr_id = find_union.find_representant(vertex_id);
    alive.erase(vertex_id);
    dead.insert(vertex_id);

    for (const auto& neighbour : graph.get_neighbours(vertex)) {
        const auto neighbour_repr_id =
            find_union.find_representant(neighbour->id);
        if (partitions[repr_id] == partitions[neighbour_repr_id]) {
            if (other_explorer.visited_scc[neighbour_repr_id] ==
                no_explorations)
                cycle_created = true;
            if (!alive_or_dead(neighbour->id))
                add_alive(neighbour->id);
        }
    }
}

void Explorer::increase_explorations_no() { no_explorations++; }

bool Explorer::process_best_alive_option(Explorer& other_explorer,
                                         const DynamicOrderPtr& order) {
    const auto x = get_best_alive_option();
    const auto x_repr = find_union.find_representant(*x);
    const auto z = other_explorer.get_best_dead_option();
    if (z.has_value()) {
        const auto z_repr = find_union.find_representant(*z);

        if (current_alive_surpassed_other_best_dead(x_repr, z_repr, order)) {
            finished_processing_alive = true;
            return true;
        }
        if (x_repr == z_repr &&
            (cycle_created || other_explorer.cycle_created)) {
            finished_processing_alive = true;
            pivot_id = z;
            return true;
        }
    }

    status[*x] = no_explorations;
    explore(*x, other_explorer);
    return false;
}

void Explorer::dfs(const VertexPtr& current,
                   const std::vector<Vertex_id_t>& permitted_components_ids,
                   std::vector<Vertex_id_t>& marked_canonical_ids) {
    visited[current->id] = no_explorations;
    const auto current_repr_id = find_union.find_representant(current->id);
    bool current_marked = contains(permitted_components_ids, current_repr_id);

    for (const auto& neighbour : graph.get_neighbours(current)) {
        const auto neighbour_repr_id =
            find_union.find_representant(neighbour->id);
        if (status[neighbour->id] == no_explorations) {
            if (visited[neighbour->id] != no_explorations)
                dfs(neighbour, permitted_components_ids, marked_canonical_ids);
            current_marked |= marked[neighbour_repr_id] == no_explorations;
        } else {
            current_marked |=
                contains(permitted_components_ids, neighbour_repr_id);
        }
    }

    if (current_marked) {
        marked_canonical_ids.emplace_back(
            find_union.find_representant(current->id));
        marked[current_repr_id] = no_explorations;
    }
}

void Explorer::extend_canonical_order(
    Vertex_id_t current_id, std::vector<Vertex_id_t>& new_canonical_order) {
    const auto current_repr_id = find_union.find_representant(current_id);
    if (reordered_component[current_repr_id] != no_explorations) {
        reordered_component[current_repr_id] = no_explorations;
        new_canonical_order.emplace_back(current_repr_id);
    }
}

void Explorer::generate_canonical_order(
    Vertex_id_t start_repr_id, std::vector<Vertex_id_t>& new_canonical_order,
    bool update_forward) {
    reordered_component[start_repr_id] = no_explorations;

    if (update_forward) {
        for (auto current_iter = dead.rbegin(); current_iter != dead.rend();
             ++current_iter)
            extend_canonical_order(*current_iter, new_canonical_order);
    } else {
        for (const auto& current_iter : dead)
            extend_canonical_order(current_iter, new_canonical_order);
    }

    dead.clear();
}

void Explorer::generate_canonical_order(
    Vertex_id_t start_repr_id, Vertex_id_t middle_id, Explorer& other_explorer,
    std::vector<Vertex_id_t>& new_canonical_order, bool update_forward,
    bool new_scc_created) {
    generate_canonical_order(start_repr_id, new_canonical_order,
                             update_forward);
    const auto middle_repr_id = find_union.find_representant(middle_id);
    if (new_scc_created)
        new_canonical_order.emplace_back(middle_repr_id);
    other_explorer.generate_canonical_order(middle_repr_id, new_canonical_order,
                                            update_forward);
}

void Explorer::clear() {
    alive.clear();
    dead.clear();
    cycle_created = false;
    pivot_id = std::nullopt;
    finished_processing_alive = false;
}

std::optional<Vertex_id_t> ForwardExplorer::get_best_alive_option() {
    return get_minimum_from_heap(alive);
}

std::optional<Vertex_id_t> ForwardExplorer::get_best_dead_option() {
    return get_maximum_from_heap(dead);
}

std::optional<Vertex_id_t> BackwardExplorer::get_best_alive_option() {
    return get_maximum_from_heap(alive);
}

std::optional<Vertex_id_t> BackwardExplorer::get_best_dead_option() {
    return get_minimum_from_heap(dead);
}

bool ForwardExplorer::current_alive_surpassed_other_best_dead(
    Vertex_id_t current_alive, Vertex_id_t other_best_dead,
    const DynamicOrderPtr& order) {
    return order->is_before(other_best_dead, current_alive);
}

bool BackwardExplorer::current_alive_surpassed_other_best_dead(
    Vertex_id_t current_alive, Vertex_id_t other_best_dead,
    const DynamicOrderPtr& order) {
    return order->is_before(current_alive, other_best_dead);
}

void SampleSearch::find_component(const VertexPtr& u, const VertexPtr& v) {
    Explorer::increase_explorations_no();
    const auto u_repr = find_representative_vertex(u);
    const auto v_repr = find_representative_vertex(v);
    if (order->is_before(u_repr->id, v_repr->id))
        return;

    forward_explorer.add_alive(v->id);
    backward_explorer.add_alive(u->id);
    const auto& partitions = partitions_handler.get_partitions();
    if (u_repr == v_repr || partitions[u_repr->id] != partitions[v_repr->id])
        return;

    while (forward_explorer.any_alive() && backward_explorer.any_alive()) {
        if (forward_explorer.process_best_alive_option(backward_explorer,
                                                       order))
            break;
        if (backward_explorer.process_best_alive_option(forward_explorer,
                                                        order))
            break;
    }

    if (!forward_explorer.cycle_found() && !backward_explorer.cycle_found())
        return;

    auto pivot_id = forward_explorer.get_pivot_id();
    if (!pivot_id.has_value())
        pivot_id = backward_explorer.get_pivot_id();
    std::vector<Vertex_id_t> permitted_components_ids;
    if (pivot_id.has_value()) {
        const auto pivot_repr_id = find_union.find_representant(*pivot_id);
        permitted_components_ids = {pivot_repr_id,
                                    find_union.find_representant(v->id)};
        backward_explorer.dfs(u, permitted_components_ids,
                              new_scc_canonical_ids);
        permitted_components_ids[1] = find_union.find_representant(u->id);
        forward_explorer.dfs(v, permitted_components_ids,
                             new_scc_canonical_ids);
    } else {
        permitted_components_ids = {find_union.find_representant(u->id)};
        forward_explorer.dfs(v, permitted_components_ids,
                             new_scc_canonical_ids);
    }
}

void SampleSearch::unionize_and_remove_non_canonical(Vertex_id_t start_id) {
    const auto start_repr_id = find_union.find_representant(start_id);
    order->insert_after(restore_canonical_order_helper, start_repr_id);

    for (size_t i = 1; i < new_scc_canonical_ids.size(); i++) {
        const auto union_result = find_union.union_elements(
            new_scc_canonical_ids[i - 1], new_scc_canonical_ids[i]);
        if (union_result.has_value()) {
            const auto [_, old_repr_id] = *union_result;
            order->remove(old_repr_id);
        }
    }
}

void SampleSearch::update_order(const VertexPtr& u, const VertexPtr& v,
                                bool update_forward) {
    const auto new_scc_created = !new_scc_canonical_ids.empty();
    if (new_scc_created) {
        if (update_forward)
            forward_explorer.erase_from_dead(v->id);
        else
            backward_explorer.erase_from_dead(u->id);
    }

    if (update_forward)
        forward_explorer.generate_canonical_order(
            restore_canonical_order_helper, v->id, backward_explorer,
            new_canonical_order, update_forward, new_scc_created);
    else
        backward_explorer.generate_canonical_order(
            restore_canonical_order_helper, u->id, forward_explorer,
            new_canonical_order, update_forward, new_scc_created);

    auto previous_id = restore_canonical_order_helper;
    for (const auto& canonical_id : new_canonical_order) {
        order->remove(canonical_id);
        if (update_forward)
            order->insert_before(canonical_id, previous_id);
        else
            order->insert_after(canonical_id, previous_id);
        previous_id = canonical_id;
    }

    order->remove(restore_canonical_order_helper);
}

void SampleSearch::algorithm_step(VertexPtr u, VertexPtr v) {
    // Phase 1 from the paper.
    sample.process_edge(u, v);
    remove_non_canonical_reached_from_sampled();
    partitions_handler.fill_up_and_down(forward_context, backward_context);
    partitions_handler.process_up_and_down();
    merge_scc_containing_sampled();
    if (!forward_context->new_scc->empty())
        return;

    // Phase 2 from the paper without point 6c which is moved to the next phase.
    find_component(u, v);

    // Phase 3 from the paper.
    const auto maximum_dead_id = forward_explorer.get_maximum_dead();
    const auto minimum_dead_id = backward_explorer.get_minimum_dead();
    if (!maximum_dead_id.has_value())
        return;

    const auto& forward_dead = forward_explorer.get_dead_as_vector();
    const auto& backward_dead = backward_explorer.get_dead_as_vector();
    const auto update_forward =
        backward_explorer.get_finished_processing_alive();

    // Phase 2 point 6c + updating the order to contain only canonical vertices.
    const auto start_id = update_forward ? maximum_dead_id : minimum_dead_id;
    unionize_and_remove_non_canonical(*start_id);

    forward_explorer.populate_dead_with_vector(forward_dead);
    backward_explorer.populate_dead_with_vector(backward_dead);
    update_order(u, v, update_forward);
}

void SampleSearch::postprocess_edge(VertexPtr u, VertexPtr v) {
    graph.add_edge(u, v);
    reversed_graph.add_edge(v, u);

    forward_context->reached->clear();
    forward_context->new_scc->clear();

    forward_explorer.clear();
    backward_explorer.clear();

    new_scc_canonical_ids.clear();
    new_canonical_order.clear();
}

void SparsifiedSampleSearch::run(const Raw_edges_list& edges) {
    for (const auto& edge : edges) {
        graph_sparsifier_forward.increase_edges_no();
        graph_sparsifier_backward.increase_edges_no();

        const auto [u_id, v_id] = edge;
        const auto u = graph.get_vertex_by_id(u_id);
        const auto v = graph.get_vertex_by_id(v_id);

        graph_sparsifier_forward.generate_new_edges(u);
        graph_sparsifier_backward.generate_new_edges(v);
        graph_sparsifier_forward.insert_generated_edge(
            graph_sparsifier_forward.get_corresponding_id(u),
            graph_sparsifier_backward.get_corresponding_id(v));

        for (const auto& [w_id, z_id] :
             *graph_sparsifier_forward.get_generated_edges()) {
            const auto w = graph.get_vertex_by_id(w_id);
            const auto z = graph.get_vertex_by_id(z_id);
            algorithm_step(w, z);
            postprocess_edge(w, z);
        }

        graph_sparsifier_forward.clear_generated_edges();
    }
}
