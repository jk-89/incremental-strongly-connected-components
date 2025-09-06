#ifndef DYNAMIC_ORDER_HPP
#define DYNAMIC_ORDER_HPP

#include <list>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "find_union.hpp"
#include "rng.hpp"

using Element_t = size_t;
using Element_list = std::list<Element_t>;

// Interface of a data structure that represents a list of distinct integers and
// allows to:
// 1. Insert element x just before or just after element y.
// 2. Remove element x.
// 3. Ask about the relative order of element x and y.
//    (that is, whether x occurs before or after y in the list).
class DynamicOrder {
   public:
    DynamicOrder() = default;
    virtual ~DynamicOrder() = default;

    virtual void insert_back(Element_t x) = 0;
    // Inserts x before y.
    virtual void insert_before(Element_t x, Element_t y) = 0;
    // Inserts x after y.
    virtual void insert_after(Element_t x, Element_t y) = 0;

    virtual void remove(Element_t x) = 0;

    // Does x occur before y in the data structure.
    virtual bool is_before(Element_t x, Element_t y) const = 0;

    // Returns the maximum potential number of elements that could be present
    // in the list at the same time.
    virtual size_t total_elements_capacity() = 0;
    virtual void extend_elements_capacity() = 0;
    virtual Element_t first_element() = 0;
};

using DynamicOrderPtr = std::shared_ptr<DynamicOrder>;

struct DynamicOrderComparator {
    const DynamicOrderPtr dynamic_order;

    explicit DynamicOrderComparator(DynamicOrderPtr dynamic_order)
        : dynamic_order(std::move(dynamic_order)) {}

    bool operator()(Element_t x, Element_t y) const {
        return dynamic_order->is_before(x, y);
    }
};

struct FindUnionDynamicOrderComparator {
    const DynamicOrderPtr dynamic_order;
    std::reference_wrapper<const FindUnion> find_union;

    FindUnionDynamicOrderComparator(DynamicOrderPtr dynamic_order,
                                    const FindUnion& find_union)
        : dynamic_order(std::move(dynamic_order)), find_union(find_union) {}

    bool operator()(Element_t x, Element_t y) const {
        const auto x_repr = find_union.get().find_representant(x);
        const auto y_repr = find_union.get().find_representant(y);
        if (x_repr == y_repr)
            return x < y;
        return dynamic_order->is_before(x_repr, y_repr);
    }
};

// Trivial implementation on list, operation 3 has pessimistic O(n) complexity.
class DynamicOrderBasicList : public DynamicOrder {
   private:
    Element_list order;
    std::vector<std::optional<Element_list::iterator>> element_pointers;

   public:
    explicit DynamicOrderBasicList(size_t no_elements)
        : element_pointers(no_elements) {
        for (Element_t i = 0; i < no_elements; i++)
            DynamicOrderBasicList::insert_back(i);
    }

    void insert_back(Element_t x) override;
    void insert_before(Element_t x, Element_t y) override;
    void insert_after(Element_t x, Element_t y) override;

    void remove(Element_t x) override;

    bool is_before(Element_t x, Element_t y) const override;

    size_t total_elements_capacity() override;
    void extend_elements_capacity() override;
    Element_t first_element() override;
};

class DynamicOrderTreap : public DynamicOrder {
   private:
    struct Node {
        Element_t value;
        size_t priority{}, size;
        Node *left, *right, *parent;

        explicit Node(Element_t value)
            : value(value),
              priority(RNG::instance().randint()),
              size(1),
              left(nullptr),
              right(nullptr),
              parent(nullptr) {}
    };

    Node* root;
    std::vector<std::optional<Node*>> nodes;

    static size_t get_size(const Node* node);
    static size_t get_rank(const Node* node);

    static void update(Node* node);
    static void split(Node* node, size_t k, Node*& left, Node*& right);
    static Node* merge(Node* x, Node* y);

    void insert_at_position(Element_t x, size_t position);

   public:
    explicit DynamicOrderTreap(size_t no_elements)
        : root(nullptr), nodes(no_elements) {
        for (size_t i = 0; i < no_elements; i++)
            DynamicOrderTreap::insert_back(i);
    }

    ~DynamicOrderTreap() override {
        for (auto& node : nodes) {
            if (node) {
                delete *node;
                node = std::nullopt;
            }
        }
    }

    void insert_back(Element_t x) override;
    void insert_before(Element_t x, Element_t y) override;
    void insert_after(Element_t x, Element_t y) override;

    void remove(Element_t x) override;

    bool is_before(Element_t x, Element_t y) const override;

    size_t total_elements_capacity() override;
    void extend_elements_capacity() override;
    Element_t first_element() override;
};

class DynamicOrderList : public DynamicOrder {
   private:
    using Label_t = size_t;
    constexpr static Label_t LOG_MAX = 62;
    constexpr static Label_t MAX_LABEL = 1LL << LOG_MAX;

    static size_t ceil(size_t x, size_t y) { return (x + y - 1) / y; }

    struct UpperNode : std::enable_shared_from_this<UpperNode> {
        Label_t label;
        std::shared_ptr<UpperNode> next;
        std::weak_ptr<UpperNode> prev;

        UpperNode(Label_t label, std::shared_ptr<UpperNode> next,
                  const std::shared_ptr<UpperNode>& prev)
            : label(label), next(std::move(next)), prev(prev) {}

        std::shared_ptr<UpperNode> insert_after();
        void remove();

        int compare(const std::shared_ptr<UpperNode>& other) const;
    };

    struct LowerNode : std::enable_shared_from_this<LowerNode> {
        constexpr static Label_t MIN_STEP_SIZE = MAX_LABEL / (LOG_MAX + 1);

        Label_t label;
        std::shared_ptr<LowerNode> next;
        std::weak_ptr<LowerNode> prev;
        std::shared_ptr<UpperNode> parent;
        Element_t value;

        LowerNode(Label_t label, std::shared_ptr<LowerNode> next,
                  const std::shared_ptr<LowerNode>& prev,
                  const std::shared_ptr<UpperNode>& parent, Element_t value)
            : label(label),
              next(std::move(next)),
              prev(prev),
              parent(parent),
              value(value) {}

        std::shared_ptr<LowerNode> insert_after(Element_t next_value);
        void remove() const;

        int compare(const std::shared_ptr<LowerNode>& other) const;
    };

    std::shared_ptr<LowerNode> head, tail;
    std::vector<std::optional<std::shared_ptr<LowerNode>>> nodes;

   public:
    explicit DynamicOrderList(size_t no_elements) : nodes(no_elements) {
        const auto upper_head =
            std::make_shared<UpperNode>(0, nullptr, nullptr);
        const auto upper_tail =
            std::make_shared<UpperNode>(MAX_LABEL - 1, nullptr, upper_head);
        upper_head->next = upper_tail;

        head = std::make_shared<LowerNode>(0, nullptr, nullptr, upper_head, 0);
        tail = std::make_shared<LowerNode>(MAX_LABEL - 1, nullptr, head,
                                           upper_tail, 0);
        head->next = tail;

        for (size_t i = no_elements; i > 0; i--)
            nodes[i - 1] = head->insert_after(i - 1);
    }

    ~DynamicOrderList() override {
        auto curr_node = head;
        while (curr_node) {
            const auto next = curr_node->next;
            curr_node->prev.reset();
            curr_node->next = nullptr;
            curr_node->parent = nullptr;
            curr_node = next;
        }
    }

    void insert_back(Element_t x) override;
    void insert_before(Element_t x, Element_t y) override;
    void insert_after(Element_t x, Element_t y) override;

    void remove(Element_t x) override;

    bool is_before(Element_t x, Element_t y) const override;

    size_t total_elements_capacity() override;
    void extend_elements_capacity() override;
    Element_t first_element() override;
};

#endif  // DYNAMIC_ORDER_HPP
