#include <iostream>
#include <mutex>
#include <stdexcept>
#include <utility>
#include <shared_mutex>

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

    Node* head_;
    Node* tail_;
    size_t size_;
    mutable std::shared_mutex mut_;

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

    void pop_front_ul() {
        if (head_->next == tail_) return;
        Node* victim = head_->next;
        head_->next = victim->next;
        victim->next->prev = head_;
        delete victim;
        --size_;
    }

    void pop_back_ul() {
        if (tail_->prev == head_) return;
        Node* victim = tail_->prev;
        tail_->prev = victim->prev;
        victim->prev->next = tail_;
        delete victim;
        --size_;
    }

public:
    Deque() : head_(new Node()), tail_(new Node()), size_(0) {
        head_->next = tail_;
        tail_->prev = head_;
    }

    Deque(const Deque&) = delete;
    Deque& operator=(const Deque&) = delete;

    Deque(Deque&& other) noexcept {
        std::unique_lock lk_other(other.mut_);
        head_ = std::exchange(other.head_, nullptr);
        tail_ = std::exchange(other.tail_, nullptr);
        size_ = std::exchange(other.size_, 0);
    }

    Deque& operator=(Deque&& other) noexcept {
        if (this != &other) {
            std::scoped_lock lk(mut_, other.mut_);
            while (head_ && head_->next && head_->next != tail_) pop_front_ul();
            delete head_;
            delete tail_;
            head_ = std::exchange(other.head_, nullptr);
            tail_ = std::exchange(other.tail_, nullptr);
            size_ = std::exchange(other.size_, 0);
        }
        return *this;
    }

    void push_front(const T& value) {
        std::unique_lock lk(mut_);
        emplace_front(value);
    }

    void push_back(const T& value) {
        std::unique_lock lk (mut_);
        emplace_back(value);
    }

    void pop_front() {
        std::unique_lock lk(mut_);
        pop_front_ul();
    }

    void pop_back() {
        std::unique_lock lk(mut_);
        pop_back_ul();
    }

    [[nodiscard]] T& front() {
        std::shared_lock lk (mut_);
        if (head_->next != tail_) throw std::runtime_error("Deque is empty.");
        return head_->next->value;
    }

    [[nodiscard]] const T& front() const {
        std::shared_lock lk (mut_);
        if (head_->next != tail_) throw std::runtime_error("Deque is empty.");
        return head_->next->value;
    }

    [[nodiscard]] T& back() {
        std::shared_lock lk (mut_);
        if (head_->next == tail_) throw std::runtime_error("Deque is empty.");
        return tail_->prev->value;
    }

    [[nodiscard]] const T& back() const {
        std::shared_lock lk (mut_);
        if (head_->next == tail_) throw std::runtime_error("Deque is empty.");
        return tail_->prev->value;
    }

    void clear() {
        std::unique_lock lk (mut_);
        while (head_->next != tail_) {  // don't call empty()
            pop_front_ul();
        }
    }

    [[nodiscard]] bool empty() const noexcept { std::shared_lock lk (mut_); return size_ == 0; }
    [[nodiscard]] size_t get_size() const noexcept { std::shared_lock lk (mut_); return size_; }

    ~Deque() {
        while (head_ && head_->next && head_->next != tail_) pop_front_ul();
        if (head_) delete head_;
        if (tail_) delete tail_;
    }
};
