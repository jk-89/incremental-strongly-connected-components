#ifndef SAMPLE_SEARCH_HPP
#define SAMPLE_SEARCH_HPP

/// Implements algorithm from https://doi.org/10.4230/LIPIcs.ESA.2021.14.
/// Works in expected total time ~O(m^(4/3)).

#include <map>
#include <set>

#include "utils/algorithm.hpp"
#include "utils/dynamic_order.hpp"
#include "utils/graph_sparsifier.hpp"

struct SampleTraversalContext {
    Graph &graph;
    std::shared_ptr<std::vector<VertexPtr>> reached;
    std::vector<size_t> reached_counter;
    std::shared_ptr<std::vector<VertexPtr>> new_scc;
};

using SampleTraversalContextPtr = std::shared_ptr<SampleTraversalContext>;

class SampledVertexTraversal {
   private:
    VertexPtr root;
    std::vector<bool> visited;
    SampleTraversalContextPtr context;

    void dfs(const VertexPtr &start,
             const SampledVertexTraversal &other_traversal);

   public:
    SampledVertexTraversal(const VertexPtr &root,
                           const SampleTraversalContextPtr &context)
        : root(root), context(context) {
        const auto no_vertices = context->graph.get_no_vertices();
        visited.resize(no_vertices);
        visited[root->id] = true;
        context->reached->emplace_back(root);
        context->reached_counter[root->id] = 1;
    }

    void search_for_new_reachable(
        const VertexPtr &u, const VertexPtr &v,
        const SampledVertexTraversal &other_traversal);
};

// Responsible for handling a vertex sampled to set called S in the paper.
class SampledVertex {
   private:
    SampledVertexTraversal forward;
    SampledVertexTraversal backward;

   public:
    SampledVertex(const VertexPtr &root,
                  const SampleTraversalContextPtr &forward_context,
                  const SampleTraversalContextPtr &backward_context)
        : forward(root, forward_context), backward(root, backward_context) {}
    ~SampledVertex() = default;

    void update_reachable(const VertexPtr &u, const VertexPtr &v);
};

class Sample {
   private:
    static constexpr size_t SAMPLE_SIZE = 2;
    std::vector<SampledVertex> sampled_vertices;

   public:
    Sample(Graph &graph, std::optional<size_t> original_no_vertices,
           const SampleTraversalContextPtr &forward_context,
           const SampleTraversalContextPtr &backward_context);

    size_t size() const;

    void process_edge(const VertexPtr &u, const VertexPtr &v);
};

// Represents partition V_{i, j} from the paper.
struct Partition {
    size_t ancestors_no = 0;
    size_t descendants_no = 0;

    bool operator==(const Partition &) const = default;

    bool operator<(const Partition &other) const {
        return ancestors_no < other.ancestors_no ||
               (ancestors_no == other.ancestors_no &&
                descendants_no > other.descendants_no);
    }
};

// Handles operations on heaps called UP and DOWN in the paper.
class PartitionHeaps {
   protected:
    using Heap_t = std::set<Vertex_id_t, DynamicOrderComparator>;
    using Heap_iterator = Heap_t::iterator;
    DynamicOrderComparator order_comparator;
    std::map<Partition, Heap_t> heaps;
    std::vector<Partition> modified_heaps;

    virtual Heap_iterator get_best_option(const Partition &partition) = 0;
    virtual void update_order(const DynamicOrderPtr &order,
                              Vertex_id_t vertex_id, Vertex_id_t dummy_id) = 0;

   public:
    explicit PartitionHeaps(const DynamicOrderPtr &order)
        : order_comparator(order) {}
    virtual ~PartitionHeaps() = default;

    const std::vector<Partition> &get_modified_heaps() const {
        return modified_heaps;
    }

    void clear_modified_heaps();

    void create_empty_set(const Partition &partition);
    void insert(const Partition &partition, const VertexPtr &u);

    void process_heap(const DynamicOrderPtr &order, const Partition &partition,
                      Vertex_id_t dummy_id);
};

class PartitionHeapsUp : public PartitionHeaps {
   protected:
    Heap_iterator get_best_option(const Partition &partition) override;
    void update_order(const DynamicOrderPtr &order, Vertex_id_t vertex_id,
                      Vertex_id_t dummy_id) override;

   public:
    explicit PartitionHeapsUp(const DynamicOrderPtr &order)
        : PartitionHeaps(order) {}
};

class PartitionHeapsDown : public PartitionHeaps {
   protected:
    Heap_iterator get_best_option(const Partition &partition) override;
    void update_order(const DynamicOrderPtr &order, Vertex_id_t vertex_id,
                      Vertex_id_t dummy_id) override;

   public:
    explicit PartitionHeapsDown(const DynamicOrderPtr &order)
        : PartitionHeaps(order) {}
};

// Handles partitions - dummy nodes, sets UP, DOWN described in the paper, etc.
class PartitionsHandler {
   private:
    DynamicOrderPtr order;
    // For each vertex stores its current partition
    std::vector<Partition> partitions;
    // Assumption: for each partition dummy nodes have consecutive ids.
    std::map<Partition, Vertex_id_t> dummy_ids{};
    PartitionHeapsUp up;
    PartitionHeapsDown down;

    std::optional<Partition> next_dummy(const Partition &partition);

   public:
    PartitionsHandler(size_t no_vertices, const DynamicOrderPtr &order)
        : order(order), partitions(no_vertices), up(order), down(order) {
        // Corner case managed by hand: dummy node for partition (0, 0).
        insert_dummy({0, 0});
        const auto dummy_id_front = get_dummy_ids({0, 0}).first;
        order->remove(dummy_id_front);
        order->insert_before(dummy_id_front, order->first_element());
    }

    const std::vector<Partition> &get_partitions() const { return partitions; }

    // For a given partition returns ids of its both dummy nodes.
    std::pair<Vertex_id_t, Vertex_id_t> get_dummy_ids(
        const Partition &partition) const;
    void insert_dummy(const Partition &partition);

    void fill_up_and_down(const SampleTraversalContextPtr &forward_context,
                          const SampleTraversalContextPtr &backward_context);
    void process_up_and_down();
};

// Used to implement exploring described in the 2nd and 3rd Phase in the paper.
class Explorer {
   protected:
    using Vertex_heap = std::set<Vertex_id_t, FindUnionDynamicOrderComparator>;
    Vertex_heap alive, dead;
    Graph &graph;
    const std::vector<Partition> &partitions;
    const FindUnion &find_union;
    bool cycle_created = false;
    static std::vector<size_t> status;
    std::vector<size_t> visited_scc;
    static size_t no_explorations;
    std::vector<size_t> visited;
    std::vector<size_t> marked;
    // Needed to check condition from the 6th step of Phase 2.
    std::optional<Vertex_id_t> pivot_id = std::nullopt;
    // Needed to know whether to run UpdateForward or UpdateBackward later.
    bool finished_processing_alive = false;
    std::vector<size_t> reordered_component;

    bool alive_or_dead(Vertex_id_t vertex_id) const;

    void explore(Vertex_id_t vertex_id, const Explorer &other_explorer);

    static std::optional<Vertex_id_t> get_minimum_from_heap(
        const Vertex_heap &heap);
    static std::optional<Vertex_id_t> get_maximum_from_heap(
        const Vertex_heap &heap);

    virtual std::optional<Vertex_id_t> get_best_alive_option() = 0;
    virtual std::optional<Vertex_id_t> get_best_dead_option() = 0;

    virtual bool current_alive_surpassed_other_best_dead(
        Vertex_id_t current_alive, Vertex_id_t other_best_dead,
        const DynamicOrderPtr &order) = 0;

    void extend_canonical_order(Vertex_id_t current_id,
                                std::vector<Vertex_id_t> &new_canonical_order);

    void generate_canonical_order(Vertex_id_t start_repr_id,
                                  std::vector<Vertex_id_t> &new_canonical_order,
                                  bool update_forward);

   public:
    Explorer(const DynamicOrderPtr &order, Graph &graph,
             const std::vector<Partition> &partitions,
             const FindUnion &find_union)
        : alive(FindUnionDynamicOrderComparator(order, find_union)),
          dead(FindUnionDynamicOrderComparator(order, find_union)),
          graph(graph),
          partitions(partitions),
          find_union(find_union),
          visited_scc(graph.get_no_vertices()),
          visited(graph.get_no_vertices()),
          marked(graph.get_no_vertices()),
          reordered_component(order->total_elements_capacity() + 1) {
        status.resize(graph.get_no_vertices());
        no_explorations = 0;
    }

    virtual ~Explorer() = default;

    bool cycle_found() const { return cycle_created; }
    std::optional<Vertex_id_t> get_pivot_id() const { return pivot_id; }
    bool get_finished_processing_alive() const {
        return finished_processing_alive;
    }

    std::vector<Vertex_id_t> get_dead_as_vector();
    void populate_dead_with_vector(const std::vector<Vertex_id_t> &dead_vec);
    void erase_from_dead(Vertex_id_t vertex_id);

    bool any_alive();
    void add_alive(Vertex_id_t vertex_id);
    std::optional<Vertex_id_t> get_maximum_dead() const;
    std::optional<Vertex_id_t> get_minimum_dead() const;

    static void increase_explorations_no();

    // Returns if loop processing alive options should be terminated.
    bool process_best_alive_option(Explorer &other_explorer,
                                   const DynamicOrderPtr &order);

    void dfs(const VertexPtr &current,
             const std::vector<Vertex_id_t> &permitted_components_ids,
             std::vector<Vertex_id_t> &marked_canonical_ids);

    // If `update_forward = true` then UpdateForward from the paper is executed.
    // Otherwise, UpdateBackward is executed.
    void generate_canonical_order(Vertex_id_t start_repr_id,
                                  Vertex_id_t middle_id,
                                  Explorer &other_explorer,
                                  std::vector<Vertex_id_t> &new_canonical_order,
                                  bool update_forward, bool new_scc_created);

    void clear();
};

class ForwardExplorer : public Explorer {
   protected:
    std::optional<Vertex_id_t> get_best_alive_option() override;
    std::optional<Vertex_id_t> get_best_dead_option() override;

    bool current_alive_surpassed_other_best_dead(
        Vertex_id_t current_alive, Vertex_id_t other_best_dead,
        const DynamicOrderPtr &order) override;

   public:
    ForwardExplorer(const DynamicOrderPtr &order, Graph &graph,
                    const std::vector<Partition> &partitions,
                    const FindUnion &find_union)
        : Explorer(order, graph, partitions, find_union) {}
};

class BackwardExplorer : public Explorer {
   protected:
    std::optional<Vertex_id_t> get_best_alive_option() override;
    std::optional<Vertex_id_t> get_best_dead_option() override;

    bool current_alive_surpassed_other_best_dead(
        Vertex_id_t current_alive, Vertex_id_t other_best_dead,
        const DynamicOrderPtr &order) override;

   public:
    BackwardExplorer(const DynamicOrderPtr &order, Graph &graph,
                     const std::vector<Partition> &partitions,
                     const FindUnion &find_union)
        : Explorer(order, graph, partitions, find_union) {}
};

class SampleSearch : public Algorithm {
   private:
    Graph reversed_graph;
    SampleTraversalContextPtr forward_context;
    SampleTraversalContextPtr backward_context;
    Sample sample;
    DynamicOrderPtr order;
    PartitionsHandler partitions_handler;
    ForwardExplorer forward_explorer;
    BackwardExplorer backward_explorer;
    std::vector<Vertex_id_t> new_scc_canonical_ids;
    std::vector<Vertex_id_t> new_canonical_order;
    Vertex_id_t restore_canonical_order_helper;

    void remove_non_canonical_reached_from_sampled();
    void merge_scc_containing_sampled();

    void find_component(const VertexPtr &u, const VertexPtr &v);

    void unionize_and_remove_non_canonical(Vertex_id_t start_id);
    // Execute UpdateForward / UpdateBackward from the 3rd Phase.
    void update_order(const VertexPtr &u, const VertexPtr &v,
                      bool update_forward);

   protected:
    void postprocess_edge(VertexPtr u, VertexPtr v) override;
    void algorithm_step(VertexPtr u, VertexPtr v) override;

   public:
    SampleSearch(size_t no_vertices, DynamicOrderPtr order,
                 std::optional<size_t> original_no_vertices = std::nullopt)
        : Algorithm(no_vertices),
          reversed_graph(graph),
          // Context objects will have a shared `reached` and `new_scc` vectors.
          forward_context(
              std::make_shared<SampleTraversalContext>(SampleTraversalContext{
                  graph, std::make_shared<std::vector<VertexPtr>>(),
                  std::vector<size_t>(no_vertices),
                  std::make_shared<std::vector<VertexPtr>>()})),
          backward_context(
              std::make_shared<SampleTraversalContext>(SampleTraversalContext{
                  reversed_graph, forward_context->reached,
                  std::vector<size_t>(no_vertices), forward_context->new_scc})),
          sample(graph, original_no_vertices, forward_context,
                 backward_context),
          order(std::move(order)),
          partitions_handler(no_vertices, this->order),
          forward_explorer(this->order, graph,
                           partitions_handler.get_partitions(), find_union),
          backward_explorer(this->order, reversed_graph,
                            partitions_handler.get_partitions(), find_union) {
        restore_canonical_order_helper = this->order->total_elements_capacity();
        this->order->extend_elements_capacity();
        partitions_handler.fill_up_and_down(forward_context, backward_context);
        partitions_handler.process_up_and_down();
        forward_context->reached->clear();
    }
};

// The paper uses an assumption that each vertex has degree O(m / n).
// The procedure to justify this assumption is used here.
class SparsifiedSampleSearch : public SampleSearch {
   private:
    SimpleGraphSparsifierForward graph_sparsifier_forward;
    SimpleGraphSparsifierBackward graph_sparsifier_backward;

   public:
    SparsifiedSampleSearch(size_t no_vertices, size_t original_no_vertices,
                           DynamicOrderPtr order)
        : SampleSearch(no_vertices, std::move(order), original_no_vertices),
          graph_sparsifier_forward(original_no_vertices),
          graph_sparsifier_backward(original_no_vertices,
                                    graph_sparsifier_forward) {}

    void run(const Raw_edges_list &edges) override;
};

#endif  // SAMPLE_SEARCH_HPP
