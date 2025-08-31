#include <iostream>
#include <stdexcept>
#include <iterator>
#include <utility>

template <typename T>
class Deque {
private:
    struct Node {
        T value;
        Node* next;
        Node* prev;
        explicit Node(const T& val) : value(val), prev(nullptr), next(nullptr) {}
        Node() : value(T()), next(nullptr), prev(nullptr) {}

        Node(const Node&) = delete;
        Node& operator=(const Node&) = delete;
        Node(Node&&) = delete;
        Node& operator=(Node&&) = delete;
    };

    template <typename ValueType, typename Pointer, typename Reference, typename NodePtr, typename Dir>
    class base_iterator {
    private:
        NodePtr current_;
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = ValueType;
        using difference_type   = std::ptrdiff_t;
        using pointer           = Pointer;
        using reference         = Reference;

        explicit base_iterator(NodePtr node) : current_(node) {}

        reference operator*() const { return current_->value; }
        pointer operator->() const { return &(current_->value); }

        base_iterator& operator++() { current_ = Dir::next(current_); return *this; }
        base_iterator& operator--() { current_ = Dir::prev(current_); return *this; }

        bool operator!=(const base_iterator& other) const { return current_ != other.current_; }
        bool operator==(const base_iterator& other) const { return current_ == other.current_; }
    };

    struct Forward {
        static Node* next(Node* n) { return n->next; }
        static Node* prev(Node* n) { return n->prev; }
    };

    struct Backward {
        static Node* next(Node* n) { return n->prev; }
        static Node* prev(Node* n) { return n->next; }
    };

    using iterator = base_iterator<T, T*, T&, Node*, Forward>;
    using const_iterator = base_iterator<T, const T*, const T&, const Node*, Forward>;
    using reverse_iterator = base_iterator<T, T*, T&, Node*, Backward>;

    Node* head_;
    Node* tail_;
    size_t size_;

public:
    Deque(const Deque&) = delete;
    Deque& operator=(const Deque&) = delete;

    Deque(Deque&& other) noexcept
        : head_(std::exchange(other.head_, nullptr)),
          tail_(std::exchange(other.tail_, nullptr)),
          size_(std::exchange(other.size_, 0)) {}

    Deque& operator=(Deque&& other) noexcept {
        if (this != &other) {
            clear();
            delete head_;
            delete tail_;
            head_ = std::exchange(other.head_, nullptr);
            tail_ = std::exchange(other.tail_, nullptr);
            size_ = std::exchange(other.size_, 0);
        }
        return *this;
    }

    Deque() : head_(new Node()), tail_(new Node()), size_(0) {
        head_->next = tail_;
        tail_->prev = head_;
    }

    void push_front(const T& value) {
        emplace_front(value);
    }

    void push_back(const T& value) {
        emplace_back(value);
    }

    template<typename... Args>
    void emplace_front(Args&&... args) {
        Node* temp = head_->next;
        Node* newNode = new Node(T(std::forward<Args>(args)...));
        head_->next = newNode;
        temp->prev = newNode;
        newNode->next = temp;
        newNode->prev = head_;
        ++size_;
    }

    template<typename... Args>
    void emplace_back(Args&&... args) {
        Node* temp = tail_->prev;
        Node* newNode = new Node(T(std::forward<Args>(args)...));
        tail_->prev = newNode;
        temp->next = newNode;
        newNode->next = tail_;
        newNode->prev = temp;
        ++size_;
    }

    void pop_front() {
        if (head_->next == tail_) return;
        Node* victim = head_->next;
        head_->next = victim->next;
        victim->next->prev = head_;
        delete victim;
        --size_;
    }

    void pop_back() {
        if (tail_->prev == head_) return;
        Node* victim = tail_->prev;
        tail_->prev = victim->prev;
        victim->prev->next = tail_;
        delete victim;
        --size_;
    }

    [[nodiscard]] T& front() {
        if (empty()) throw std::runtime_error("Deque is empty.");
        return head_->next->value;
    }

    [[nodiscard]] const T& front() const {
        if (empty()) throw std::runtime_error("Deque is empty.");
        return head_->next->value;
    }

    [[nodiscard]] T& back() {
        if (empty()) throw std::runtime_error("Deque is empty.");
        return tail_->prev->value;
    }

    [[nodiscard]] const T& back() const {
        if (empty()) throw std::runtime_error("Deque is empty.");
        return tail_->prev->value;
    }

    void clear() {
        while (!empty()) pop_front();
    }

    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }
    [[nodiscard]] size_t get_size() const noexcept { return size_; }

    [[nodiscard]] iterator begin() { return iterator(head_->next); }
    [[nodiscard]] iterator end() { return iterator(tail_); }
    [[nodiscard]] const_iterator begin() const { return const_iterator(head_->next); }
    [[nodiscard]] const_iterator end() const { return const_iterator(tail_); }

    [[nodiscard]] reverse_iterator rbegin() { return reverse_iterator(tail_->prev); }
    [[nodiscard]] reverse_iterator rend() { return reverse_iterator(head_); }

    ~Deque() {
        clear();
        if (head_) delete head_;
        if (tail_) delete tail_;
    }
};

template<class T, class Container = Deque<T>>
class Queue {
protected:
    Container c;  

public:
    void push(const T& value) { c.push_back(value); }
    void pop() { c.pop_front(); }

    T& front() { return c.front(); }
    const T& front() const { return c.front(); }
    bool empty() { return c.empty(); }
    size_t size() const { return c.get_size(); }
};
