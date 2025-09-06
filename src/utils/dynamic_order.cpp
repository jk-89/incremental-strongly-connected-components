#include "dynamic_order.hpp"

#include <iostream>
#include <stdexcept>

namespace {
template <typename T>
void assure_element_exists(Element_t x,
                           const std::vector<std::optional<T>>& elements) {
    if (!elements[x].has_value())
        throw std::invalid_argument("Element does not exist: " +
                                    std::to_string(x));
}

template <typename T>
void assure_element_does_not_exist(
    Element_t x, const std::vector<std::optional<T>>& elements) {
    if (elements[x].has_value())
        throw std::invalid_argument("Element already exists: " +
                                    std::to_string(x));
}
}  // namespace

void DynamicOrderBasicList::insert_back(Element_t x) {
    assure_element_does_not_exist(x, element_pointers);
    order.push_back(x);
    element_pointers[x] = std::prev(order.end());
}

void DynamicOrderBasicList::insert_before(Element_t x, Element_t y) {
    assure_element_does_not_exist(x, element_pointers);
    assure_element_exists(y, element_pointers);
    element_pointers[x] = order.insert(*element_pointers[y], x);
}

void DynamicOrderBasicList::insert_after(Element_t x, Element_t y) {
    assure_element_does_not_exist(x, element_pointers);
    assure_element_exists(y, element_pointers);
    element_pointers[x] = order.insert(std::next(*element_pointers[y]), x);
}

void DynamicOrderBasicList::remove(Element_t x) {
    assure_element_exists(x, element_pointers);
    order.erase(*element_pointers[x]);
    element_pointers[x] = std::nullopt;
}

bool DynamicOrderBasicList::is_before(Element_t x, Element_t y) const {
    assure_element_exists(x, element_pointers);
    assure_element_exists(y, element_pointers);

    auto iter = *element_pointers[x];
    while (++iter != order.end()) {
        if (*iter == y)
            return true;
    }
    return false;
}

size_t DynamicOrderBasicList::total_elements_capacity() {
    return element_pointers.size();
}

void DynamicOrderBasicList::extend_elements_capacity() {
    element_pointers.emplace_back(std::nullopt);
}

Element_t DynamicOrderBasicList::first_element() {
    if (order.empty())
        throw std::runtime_error(
            "Cannot retrieve the first element from an empty container.");
    return *order.begin();
}

size_t DynamicOrderTreap::get_size(const Node* node) {
    return node ? node->size : 0;
}

size_t DynamicOrderTreap::get_rank(const Node* node) {
    auto rank = get_size(node->left) + 1;
    while (node->parent) {
        if (node == node->parent->right)
            rank += get_size(node->parent->left) + 1;
        node = node->parent;
    }

    return rank;
}

void DynamicOrderTreap::update(Node* node) {
    node->size = 1 + get_size(node->left) + get_size(node->right);
    if (node->left)
        node->left->parent = node;
    if (node->right)
        node->right->parent = node;
}

void DynamicOrderTreap::split(Node* node, size_t k, Node*& left, Node*& right) {
    if (!node) {
        left = right = nullptr;
        return;
    }

    if (get_size(node->left) >= k) {
        split(node->left, k, left, node->left);
        if (node->left)
            node->left->parent = node;
        right = node;
        right->parent = nullptr;
        update(right);
    } else {
        split(node->right, k - get_size(node->left) - 1, node->right, right);
        if (node->right)
            node->right->parent = node;
        left = node;
        left->parent = nullptr;
        update(left);
    }
}

DynamicOrderTreap::Node* DynamicOrderTreap::merge(Node* x, Node* y) {
    if (!x || !y)
        return x ? x : y;

    if (x->priority > y->priority) {
        x->right = merge(x->right, y);
        if (x->right)
            x->right->parent = x;
        update(x);
        x->parent = nullptr;
        return x;
    } else {
        y->left = merge(x, y->left);
        if (y->left)
            y->left->parent = y;
        update(y);
        y->parent = nullptr;
        return y;
    }
}

void DynamicOrderTreap::insert_at_position(Element_t x, size_t position) {
    Node *left, *right;
    split(root, position, left, right);
    auto* node = new Node(x);
    nodes[x] = node;
    root = merge(merge(left, node), right);
}

void DynamicOrderTreap::insert_back(Element_t x) {
    assure_element_does_not_exist(x, nodes);
    auto* node = new Node(x);
    nodes[x] = node;
    root = merge(root, node);
}

void DynamicOrderTreap::insert_before(Element_t x, Element_t y) {
    assure_element_does_not_exist(x, nodes);
    assure_element_exists(y, nodes);
    insert_at_position(x, get_rank(*nodes[y]) - 1);
}

void DynamicOrderTreap::insert_after(Element_t x, Element_t y) {
    assure_element_does_not_exist(x, nodes);
    assure_element_exists(y, nodes);
    insert_at_position(x, get_rank(*nodes[y]));
}

void DynamicOrderTreap::remove(Element_t x) {
    assure_element_exists(x, nodes);
    const auto node_x = *nodes[x];
    const auto pos = get_rank(node_x) - 1;
    Node *left, *temp, *right;
    split(root, pos, left, temp);
    split(temp, 1, temp, right);
    delete node_x;
    nodes[x] = std::nullopt;
    root = merge(left, right);
}

bool DynamicOrderTreap::is_before(Element_t x, Element_t y) const {
    assure_element_exists(x, nodes);
    assure_element_exists(y, nodes);
    return get_rank(*nodes[x]) < get_rank(*nodes[y]);
}

size_t DynamicOrderTreap::total_elements_capacity() { return nodes.size(); }

void DynamicOrderTreap::extend_elements_capacity() {
    nodes.emplace_back(std::nullopt);
}

Element_t DynamicOrderTreap::first_element() {
    auto node = root;
    while (node->left)
        node = node->left;
    return node->value;
}

std::shared_ptr<DynamicOrderList::UpperNode>
DynamicOrderList::UpperNode::insert_after() {
    auto current_node = next;
    size_t j = 1;
    for (; current_node && current_node->label - label <= j * j;
         ++j, current_node = current_node->next) {
    }
    size_t mul = ceil(MAX_LABEL - 1 - label, j);
    if (current_node)
        mul = ceil(current_node->label - label, j);

    current_node = next;
    for (size_t k = 1; k < j; ++k, current_node = current_node->next)
        current_node->label = label + mul * k;

    const Label_t new_label = ceil(label + next->label, 2);
    auto result =
        std::make_shared<UpperNode>(new_label, next, shared_from_this());
    next->prev = result;
    next = result;
    return result;
}

void DynamicOrderList::UpperNode::remove() {
    const auto prev_node = prev.lock();
    if (prev_node) {
        if (next)
            next->prev = prev_node;
        prev_node->next = next;
    }
    next = nullptr;
    prev.reset();
}

int DynamicOrderList::UpperNode::compare(
    const std::shared_ptr<UpperNode>& other) const {
    if (label < other->label)
        return -1;
    if (label > other->label)
        return 1;
    return 0;
}

std::shared_ptr<DynamicOrderList::LowerNode>
DynamicOrderList::LowerNode::insert_after(Element_t next_value) {
    auto next_label = MAX_LABEL;
    const auto self = shared_from_this();
    const auto result =
        std::make_shared<LowerNode>(0, next, self, parent, next_value);
    if (next) {
        next->prev = result;
        if (parent == next->parent)
            next_label = next->label;
    }
    next = result;

    if (next_label != label + 1) {
        result->label = std::min((label + next_label) / 2, label + LOG_MAX);
        return result;
    }

    size_t nodes_with_same_parent = 1;
    auto begin = self;
    while (const auto prev_node = begin->prev.lock()) {
        if (prev_node->parent != parent)
            break;
        begin = prev_node;
        nodes_with_same_parent++;
    }
    auto end = self;
    while (end->next && end->next->parent == parent) {
        end = end->next;
        nodes_with_same_parent++;
    }
    end = end->next;

    auto current_node = begin;
    auto current_parent = parent;
    while (true) {
        const auto step_size =
            std::max(MIN_STEP_SIZE, MAX_LABEL / (nodes_with_same_parent + 1));

        size_t processed = 0;
        for (Label_t current_label = step_size;
             current_label < MAX_LABEL && processed < LOG_MAX;
             current_label += step_size, ++processed) {
            if (current_node == end)
                return result;
            current_node->label = current_label;
            current_node->parent = current_parent;
            current_node = current_node->next;
            nodes_with_same_parent--;
        }

        if (current_node == end)
            return result;
        current_parent = current_parent->insert_after();
    }
}

void DynamicOrderList::LowerNode::remove() const {
    bool unique_parent = true;
    if (next) {
        next->prev = prev;
        unique_parent = next->parent != parent;
    }
    if (const auto prev_node = prev.lock()) {
        prev_node->next = next;
        unique_parent &= prev_node->parent != parent;
    }
    if (unique_parent && parent)
        parent->remove();
}

int DynamicOrderList::LowerNode::compare(
    const std::shared_ptr<LowerNode>& other) const {
    const auto parent_compare = parent->compare(other->parent);
    if (parent_compare)
        return parent_compare;
    if (label < other->label)
        return -1;
    if (label > other->label)
        return 1;
    return 0;
}

void DynamicOrderList::insert_back(Element_t x) {
    assure_element_does_not_exist(x, nodes);
    nodes[x] = tail->prev.lock()->insert_after(x);
}

void DynamicOrderList::insert_before(Element_t x, Element_t y) {
    assure_element_does_not_exist(x, nodes);
    assure_element_exists(y, nodes);
    const auto node_y = *nodes[y];
    nodes[x] = node_y->prev.lock()->insert_after(x);
}

void DynamicOrderList::insert_after(Element_t x, Element_t y) {
    assure_element_does_not_exist(x, nodes);
    assure_element_exists(y, nodes);
    nodes[x] = (*nodes[y])->insert_after(x);
}

void DynamicOrderList::remove(Element_t x) {
    assure_element_exists(x, nodes);
    (*nodes[x])->remove();
    nodes[x] = std::nullopt;
}

bool DynamicOrderList::is_before(Element_t x, Element_t y) const {
    assure_element_exists(x, nodes);
    assure_element_exists(y, nodes);
    return (*nodes[x])->compare(*nodes[y]) < 0;
}

size_t DynamicOrderList::total_elements_capacity() { return nodes.size(); }

void DynamicOrderList::extend_elements_capacity() {
    nodes.emplace_back(std::nullopt);
}

size_t DynamicOrderList::first_element() { return head->next->value; }
