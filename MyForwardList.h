#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <algorithm>
#include <memory> // Для std::allocator_traits

// Добавил шаблонный параметр Alloc по умолчанию std::allocator<T>
template<typename T, typename Alloc = std::allocator<T>>
class MyForwardList {
public:
    struct Node {
        T data;
        Node* next;
        
        // Добавил конструктор, чтобы allocator_traits::construct мог его вызвать
        Node(const T& val, Node* nxt) : data(val), next(nxt) {}
    };

    // Определяю тип аллокатора для Node через rebind (
    using NodeAlloc = typename std::allocator_traits<Alloc>::rebind_alloc<Node>;
    using NodeAllocTraits = std::allocator_traits<NodeAlloc>;

    MyForwardList() : m_head(nullptr), m_size(0), m_alloc() {}

    // Копирующий конструктор
    MyForwardList(const MyForwardList& other)
        : m_head(nullptr), m_size(0), m_alloc(other.m_alloc) {
        Node* tail = nullptr;
        for (Node* cur = other.m_head; cur != nullptr; cur = cur->next) {
            Node* node = create_node(cur->data, nullptr);
            if (!m_head) m_head = node;
            else         tail->next = node;
            tail = node;
            ++m_size;
        }
    }

    // Move-конструктор
    MyForwardList(MyForwardList&& other) noexcept
        : m_head(other.m_head), m_size(other.m_size), m_alloc(std::move(other.m_alloc)) {
        other.m_head = nullptr;
        other.m_size = 0;
    }

    ~MyForwardList() { clear(); }

    MyForwardList& operator=(const MyForwardList& other) {
        if (this == &other) return *this;
        MyForwardList tmp(other);
        swap(tmp);
        return *this;
    }

    MyForwardList& operator=(MyForwardList&& other) noexcept {
        if (this == &other) return *this;
        clear();
        m_head = other.m_head;
        m_size = other.m_size;
        m_alloc = std::move(other.m_alloc); // Перенос аллокатор
        other.m_head = nullptr;
        other.m_size = 0;
        return *this;
    }

    void push_back(const T& value) {
        Node* node = create_node(value, nullptr); 
        if (!m_head) {
            m_head = node;
        } else {
            Node* cur = m_head;
            while (cur->next) cur = cur->next;
            cur->next = node;
        }
        ++m_size;
    }

    void insert(size_t pos, const T& value) {
        if (pos > m_size) throw std::out_of_range("MyForwardList::insert");

        if (pos == 0) {
            Node* node = create_node(value, m_head);
            m_head = node;
            ++m_size;
            return;
        }
        Node* prev = m_head;
        for (size_t i = 0; i < pos - 1; ++i) prev = prev->next;
        
        Node* node = create_node(value, prev->next);
        prev->next = node;
        ++m_size;
    }

    void erase(size_t pos) {
        if (pos >= m_size) throw std::out_of_range("MyForwardList::erase");

        if (pos == 0) {
            Node* old = m_head;
            m_head = m_head->next;
            destroy_node(old); 
            --m_size;
            return;
        }
        Node* prev = m_head;
        for (size_t i = 0; i < pos - 1; ++i) prev = prev->next;
        Node* old = prev->next;
        prev->next = old->next;
        destroy_node(old);
        --m_size;
    }

    size_t size() const { return m_size; }
    
    // Метод empty()
    bool empty() const { return m_size == 0; } 

    T& operator[](size_t i) { return node_at(i)->data; }
    const T& operator[](size_t i) const { return node_at(i)->data; }

    T& at(size_t i) {
        if (i >= m_size) throw std::out_of_range("MyForwardList::at");
        return node_at(i)->data;
    }

    struct iterator {
        Node* ptr;
        T& operator*() const { return ptr->data; }
        T* operator->() const { return &ptr->data; }
        T& get() const { return ptr->data; }
        iterator& operator++() { ptr = ptr->next; return *this; }
        iterator operator++(int) { iterator t = *this; ++ptr; return t; }
        bool operator==(const iterator& o) const { return ptr == o.ptr; }
        bool operator!=(const iterator& o) const { return ptr != o.ptr; }
    };

    struct const_iterator {
        const Node* ptr;
        const T& operator*() const { return ptr->data; }
        const T* operator->() const { return &ptr->data; }
        const T& get() const { return ptr->data; }
        const_iterator& operator++() { ptr = ptr->next; return *this; }
        const_iterator operator++(int) { const_iterator t = *this; ++ptr; return t; }
        bool operator==(const const_iterator& o) const { return ptr == o.ptr; }
        bool operator!=(const const_iterator& o) const { return ptr != o.ptr; }
    };

    iterator begin() { return iterator{m_head}; }
    iterator end()   { return iterator{nullptr}; }
    const_iterator begin() const { return const_iterator{m_head}; }
    const_iterator end()   const { return const_iterator{nullptr}; }

private:
    Node* m_head;
    size_t m_size;
    NodeAlloc m_alloc; // Храним аллокатор для узлов

    // Вспомогательный метод для создания узла
    Node* create_node(const T& value, Node* next) {
        Node* p = NodeAllocTraits::allocate(m_alloc, 1);
        NodeAllocTraits::construct(m_alloc, p, value, next);
        return p;
    }

    // Вспомогательный метод для удаления узла
    void destroy_node(Node* p) {
        NodeAllocTraits::destroy(m_alloc, p);
        NodeAllocTraits::deallocate(m_alloc, p, 1);
    }

    Node* node_at(size_t pos) const {
        if (pos >= m_size) throw std::out_of_range("MyForwardList::node_at");
        Node* cur = m_head;
        for (size_t i = 0; i < pos; ++i) cur = cur->next;
        return cur;
    }

    void clear() {
        while (m_head) {
            Node* next = m_head->next;
            destroy_node(m_head); 
            m_head = next;
        }
        m_size = 0;
    }

    void swap(MyForwardList& other) noexcept {
        std::swap(m_head, other.m_head);
        std::swap(m_size, other.m_size);
        std::swap(m_alloc, other.m_alloc); 
    }
};