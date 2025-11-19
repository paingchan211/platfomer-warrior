#pragma once

#include <functional>
#include <utility>
#include <iostream>
#include "Constants.h"

// Debug flag for iterator logging (set to false in production)
#ifndef ITERATOR_DEBUG_LOGGING
#define ITERATOR_DEBUG_LOGGING true
#endif

// Node class for doubly linked list implementation
template <typename DataType>
class DoublyLinkedNode
{
public:
    typedef DoublyLinkedNode<DataType> Node; // Type alias for node type
    static Node NIL;                         // Sentinel node representing end of list or empty list

private:
    DataType value; // Value stored in the node
    Node *next;     // Pointer to the next node in the list
    Node *previous; // Pointer to the previous node in the list

public:
    // Default constructor that initializes node with default value and NIL pointers
    DoublyLinkedNode();

    // Constructor that initializes node with a given value
    explicit DoublyLinkedNode(const DataType &aValue);

    // Move constructor that moves a value into the node
    DoublyLinkedNode(DataType &&aValue);

    // Inserts a new node before this node in the list
    void prepend(Node *newNode);

    // Inserts a new node after this node in the list
    void append(Node *newNode);

    // Returns a copy of the value stored in the node
    DataType getValue() const;

    // Returns a reference to the value stored in the node (non-const)
    DataType &getValueRef();

    // Returns a const reference to the value stored in the node
    const DataType &getValueRef() const;

    // Returns a pointer to the next node
    Node *getNext() const;

    // Returns a pointer to the previous node
    Node *getPrevious() const;

    // Sets the next node pointer
    void setNext(Node *n);

    // Sets the previous node pointer
    void setPrevious(Node *p);

    // Checks if this node is the NIL sentinel node
    bool isNIL() const;
};

// ---------- DoublyLinkedNode Implementation ----------

// Default constructor that initializes node with default value and NIL pointers
template <typename DataType>
DoublyLinkedNode<DataType>::DoublyLinkedNode()
{
    value = DataType();
    next = &NIL;
    previous = &NIL;
}

// Constructor that initializes node with a given value
template <typename DataType>
DoublyLinkedNode<DataType>::DoublyLinkedNode(const DataType &aValue)
{
    value = aValue;
    next = &NIL;
    previous = &NIL;
}

// Move constructor that moves a value into the node
template <typename DataType>
DoublyLinkedNode<DataType>::DoublyLinkedNode(DataType &&aValue)
{
    value = std::move(aValue);
    next = &NIL;
    previous = &NIL;
}

// Inserts a new node before this node in the list
template <typename DataType>
void DoublyLinkedNode<DataType>::prepend(Node *newNode)
{
    // Set new node's next pointer to this node
    newNode->next = this;

    // If this node has a previous node, update its links
    if (this->previous != &NIL)
    {
        newNode->previous = this->previous;
        this->previous->next = newNode;
    }

    // Update this node's previous pointer to point to new node
    this->previous = newNode;
}

// Inserts a new node after this node in the list
template <typename DataType>
void DoublyLinkedNode<DataType>::append(Node *newNode)
{
    // Set new node's previous pointer to this node
    newNode->previous = this;
    // If this node has a next node, update its links
    if (this->next != &NIL)
    {
        newNode->next = this->next;
        this->next->previous = newNode;
    }
    // Update this node's next pointer to point to new node
    this->next = newNode;
}

// Returns a copy of the value stored in the node
template <typename DataType>
DataType DoublyLinkedNode<DataType>::getValue() const
{
    return value;
}

// Returns a reference to the value stored in the node (non-const)
template <typename DataType>
DataType &DoublyLinkedNode<DataType>::getValueRef()
{
    return value;
}

// Returns a const reference to the value stored in the node
template <typename DataType>
const DataType &DoublyLinkedNode<DataType>::getValueRef() const
{
    return value;
}

// Returns a pointer to the next node
template <typename DataType>
typename DoublyLinkedNode<DataType>::Node *DoublyLinkedNode<DataType>::getNext() const
{
    return next;
}

// Returns a pointer to the previous node
template <typename DataType>
typename DoublyLinkedNode<DataType>::Node *DoublyLinkedNode<DataType>::getPrevious() const
{
    return previous;
}

// Sets the next node pointer
template <typename DataType>
void DoublyLinkedNode<DataType>::setNext(Node *n)
{
    next = n;
}

// Sets the previous node pointer
template <typename DataType>
void DoublyLinkedNode<DataType>::setPrevious(Node *p)
{
    previous = p;
}

// Checks if this node is the NIL sentinel node
template <typename DataType>
bool DoublyLinkedNode<DataType>::isNIL() const
{
    return this == &NIL;
}

// Static definition of the NIL sentinel node
template <typename DataType>
DoublyLinkedNode<DataType> DoublyLinkedNode<DataType>::NIL;

// Doubly linked list container class with iterator support
template <typename T>
class DoublyLinkedList
{
public:
    using Node = DoublyLinkedNode<T>; // Type alias for node type

private:
    Node *head_{&Node::NIL}; // Pointer to the first node in the list
    Node *tail_{&Node::NIL}; // Pointer to the last node in the list
    unsigned long size_{0};  // Number of elements in the list

public:
    // Iterator class template that supports both const and non-const iteration
    template <bool is_const>
    class BasicIterator
    {
        friend class DoublyLinkedList; // Allow DoublyLinkedList to access private members

    public:
        struct iterator_category // Iterator category for STL compatibility
        {
        };
        using difference_type = long; // Type for iterator difference

        // Conditional type selection based on constness
        template <bool B, typename X, typename Y>
        struct conditional
        {
            using type = X;
        };
        template <typename X, typename Y>
        struct conditional<false, X, Y>
        {
            using type = Y;
        };

        using pointer = typename conditional<is_const, const T *, T *>::type;   // Pointer type based on constness
        using reference = typename conditional<is_const, const T &, T &>::type; // Reference type based on constness

        // Default constructor that initializes iterator to NIL
        BasicIterator();
        // Constructor that initializes iterator with a node pointer
        explicit BasicIterator(Node *n);

        // Copy constructor and assignment operator
        BasicIterator(const BasicIterator &other) = default;
        BasicIterator &operator=(const BasicIterator &other) = default;

        // Dereference operator to access the value
        reference operator*() const;
        // Arrow operator to access members of the value
        pointer operator->() const;

        // Pre-increment operator (moves to next node)
        BasicIterator &operator++();
        // Post-increment operator (moves to next node)
        BasicIterator operator++(int);

        // Pre-decrement operator (moves to previous node)
        BasicIterator &operator--();
        // Post-decrement operator (moves to previous node)
        BasicIterator operator--(int);

        // Equality comparison operator
        bool operator==(const BasicIterator &other) const;
        // Inequality comparison operator
        bool operator!=(const BasicIterator &other) const;

    private:
        Node *node_; // Pointer to the current node
    };

    using iterator = BasicIterator<false>;      // Non-const iterator type
    using const_iterator = BasicIterator<true>; // Const iterator type

    // Default constructor that creates an empty list
    DoublyLinkedList() noexcept = default;

    // Move constructor that transfers ownership from another list
    DoublyLinkedList(DoublyLinkedList &&other) noexcept;
    // Move assignment operator that transfers ownership from another list
    DoublyLinkedList &operator=(DoublyLinkedList &&other) noexcept;

    // Copy constructor is deleted (not allowed)
    DoublyLinkedList(const DoublyLinkedList &) = delete;
    // Copy assignment operator is deleted (not allowed)
    DoublyLinkedList &operator=(const DoublyLinkedList &) = delete;

    // Destructor that clears the list
    ~DoublyLinkedList();

    // Checks if the list is empty
    bool empty() const noexcept;
    // Returns the number of elements in the list
    unsigned long size() const noexcept;

    // Returns an iterator to the first element (non-const)
    iterator begin() noexcept;
    // Returns an iterator to the end of the list (non-const)
    iterator end() noexcept;
    // Returns a const iterator to the first element
    const_iterator begin() const noexcept;
    // Returns a const iterator to the end of the list
    const_iterator end() const noexcept;
    // Returns a const iterator to the first element
    const_iterator cbegin() const noexcept;
    // Returns a const iterator to the end of the list
    const_iterator cend() const noexcept;

    // Returns a reference to the first element (non-const)
    T &front();
    // Returns a const reference to the first element
    const T &front() const;
    // Returns a reference to the last element (non-const)
    T &back();
    // Returns a const reference to the last element
    const T &back() const;

    // Adds an element to the front of the list (copy)
    void push_front(const T &value);
    // Adds an element to the front of the list (move)
    void push_front(T &&value);
    // Adds an element to the back of the list (copy)
    void push_back(const T &value);
    // Adds an element to the back of the list (move)
    void push_back(T &&value);
    // Removes the first element from the list
    void pop_front();
    // Removes the last element from the list
    void pop_back();

    // Inserts an element before the specified position (copy)
    iterator insert(const_iterator pos, const T &value);
    // Inserts an element before the specified position (move)
    iterator insert(const_iterator pos, T &&value);

    // Removes the element at the specified position
    iterator erase(iterator pos);

    // Removes all elements that satisfy the predicate
    template <typename Predicate>
    int removeIf(Predicate pred);

    // Applies a function to each element in the list (non-const)
    template <typename Func>
    void forEach(Func func);

    // Applies a function to each element in the list (const)
    template <typename Func>
    void forEach(Func func) const;

    // Removes all elements from the list
    void clear() noexcept;

private:
    // Helper function to add a node to the front of the list
    void push_front_node(Node *n);
    // Helper function to add a node to the back of the list
    void push_back_node(Node *n);
};

// Default constructor that initializes iterator to NIL
template <typename T>
template <bool is_const>
DoublyLinkedList<T>::BasicIterator<is_const>::BasicIterator() : node_(&Node::NIL) {}

// Constructor that initializes iterator with a node pointer
template <typename T>
template <bool is_const>
DoublyLinkedList<T>::BasicIterator<is_const>::BasicIterator(Node *n) : node_(n) {}

// Dereference operator to access the value
template <typename T>
template <bool is_const>
typename DoublyLinkedList<T>::template BasicIterator<is_const>::reference
DoublyLinkedList<T>::BasicIterator<is_const>::operator*() const
{
    return node_->getValueRef();
}

// Arrow operator to access members of the value
template <typename T>
template <bool is_const>
typename DoublyLinkedList<T>::template BasicIterator<is_const>::pointer
DoublyLinkedList<T>::BasicIterator<is_const>::operator->() const
{
    return &node_->getValueRef();
}

// Pre-increment operator (moves to next node)
template <typename T>
template <bool is_const>
typename DoublyLinkedList<T>::template BasicIterator<is_const> &
DoublyLinkedList<T>::BasicIterator<is_const>::operator++()
{
    node_ = node_->getNext();
    return *this;
}

// Post-increment operator (moves to next node)
template <typename T>
template <bool is_const>
typename DoublyLinkedList<T>::template BasicIterator<is_const>
DoublyLinkedList<T>::BasicIterator<is_const>::operator++(int)
{
    BasicIterator tmp(*this);
    ++(*this);
    return tmp;
}

// Pre-decrement operator (moves to previous node)
template <typename T>
template <bool is_const>
typename DoublyLinkedList<T>::template BasicIterator<is_const> &
DoublyLinkedList<T>::BasicIterator<is_const>::operator--()
{
    node_ = node_->getPrevious();
    return *this;
}

// Post-decrement operator (moves to previous node)
template <typename T>
template <bool is_const>
typename DoublyLinkedList<T>::template BasicIterator<is_const>
DoublyLinkedList<T>::BasicIterator<is_const>::operator--(int)
{
    BasicIterator tmp(*this);
    --(*this);
    return tmp;
}

// Equality comparison operator
template <typename T>
template <bool is_const>
bool DoublyLinkedList<T>::BasicIterator<is_const>::operator==(const BasicIterator &other) const
{
    return node_ == other.node_;
}

// Inequality comparison operator
template <typename T>
template <bool is_const>
bool DoublyLinkedList<T>::BasicIterator<is_const>::operator!=(const BasicIterator &other) const
{
    return node_ != other.node_;
}

// Move constructor that transfers ownership from another list
template <typename T>
DoublyLinkedList<T>::DoublyLinkedList(DoublyLinkedList &&other) noexcept
{
    // Transfer ownership of nodes and size
    head_ = other.head_;
    tail_ = other.tail_;
    size_ = other.size_;
    // Reset other list to empty state
    other.head_ = other.tail_ = &Node::NIL;
    other.size_ = 0;
}

// Move assignment operator that transfers ownership from another list
template <typename T>
DoublyLinkedList<T> &DoublyLinkedList<T>::operator=(DoublyLinkedList &&other) noexcept
{
    // Check for self-assignment
    if (this != &other)
    {
        // Clear this list first
        clear();
        // Transfer ownership of nodes and size
        head_ = other.head_;
        tail_ = other.tail_;
        size_ = other.size_;
        // Reset other list to empty state
        other.head_ = other.tail_ = &Node::NIL;
        other.size_ = 0;
    }
    return *this;
}

// Destructor that clears the list
template <typename T>
DoublyLinkedList<T>::~DoublyLinkedList()
{
    clear();
}

// Checks if the list is empty
template <typename T>
bool DoublyLinkedList<T>::empty() const noexcept
{
    return size_ == 0;
}

// Returns the number of elements in the list
template <typename T>
unsigned long DoublyLinkedList<T>::size() const noexcept
{
    return size_;
}

// Returns an iterator to the first element (non-const)
template <typename T>
typename DoublyLinkedList<T>::iterator DoublyLinkedList<T>::begin() noexcept
{
    return iterator(head_);
}

// Returns an iterator to the end of the list (non-const)
template <typename T>
typename DoublyLinkedList<T>::iterator DoublyLinkedList<T>::end() noexcept
{
    return iterator(&Node::NIL);
}

// Returns a const iterator to the first element
template <typename T>
typename DoublyLinkedList<T>::const_iterator DoublyLinkedList<T>::begin() const noexcept
{
    return const_iterator(head_);
}

// Returns a const iterator to the end of the list
template <typename T>
typename DoublyLinkedList<T>::const_iterator DoublyLinkedList<T>::end() const noexcept
{
    return const_iterator(&Node::NIL);
}

// Returns a const iterator to the first element
template <typename T>
typename DoublyLinkedList<T>::const_iterator DoublyLinkedList<T>::cbegin() const noexcept
{
    return const_iterator(head_);
}

// Returns a const iterator to the end of the list
template <typename T>
typename DoublyLinkedList<T>::const_iterator DoublyLinkedList<T>::cend() const noexcept
{
    return const_iterator(&Node::NIL);
}

// Returns a reference to the first element (non-const)
template <typename T>
T &DoublyLinkedList<T>::front()
{
    // Throw exception if list is empty
    if (head_->isNIL())
        throw std::runtime_error("DoublyLinkedList::front on empty list");
    return head_->getValueRef();
}

// Returns a const reference to the first element
template <typename T>
const T &DoublyLinkedList<T>::front() const
{
    // Throw exception if list is empty
    if (head_->isNIL())
        throw std::runtime_error("DoublyLinkedList::front on empty list");
    return head_->getValueRef();
}

// Returns a reference to the last element (non-const)
template <typename T>
T &DoublyLinkedList<T>::back()
{
    // Throw exception if list is empty
    if (tail_->isNIL())
        throw std::runtime_error("DoublyLinkedList::back on empty list");
    return tail_->getValueRef();
}

// Returns a const reference to the last element
template <typename T>
const T &DoublyLinkedList<T>::back() const
{
    // Throw exception if list is empty
    if (tail_->isNIL())
        throw std::runtime_error("DoublyLinkedList::back on empty list");
    return tail_->getValueRef();
}

// Adds an element to the front of the list (copy)
template <typename T>
void DoublyLinkedList<T>::push_front(const T &value)
{
    Node *n = new Node(value);
    push_front_node(n);
}

// Adds an element to the front of the list (move)
template <typename T>
void DoublyLinkedList<T>::push_front(T &&value)
{
    Node *n = new Node(std::move(value));
    push_front_node(n);
}

// Adds an element to the back of the list (copy)
template <typename T>
void DoublyLinkedList<T>::push_back(const T &value)
{
    Node *n = new Node(value);
    push_back_node(n);
}

// Adds an element to the back of the list (move)
template <typename T>
void DoublyLinkedList<T>::push_back(T &&value)
{
    Node *n = new Node(std::move(value));
    push_back_node(n);
}

// Removes the first element from the list
template <typename T>
void DoublyLinkedList<T>::pop_front()
{
    // Return early if list is empty
    if (head_->isNIL())
        return;

    // Save head node and move head to next node
    Node *n = head_;
    head_ = head_->getNext();

    // Update links: if new head exists, set its previous to NIL, otherwise set tail to NIL
    if (!head_->isNIL())
        head_->setPrevious(&Node::NIL);
    else
        tail_ = &Node::NIL;

    // Delete the old head node and decrement size
    delete n;
    --size_;
}

// Removes the last element from the list
template <typename T>
void DoublyLinkedList<T>::pop_back()
{
    // Return early if list is empty
    if (tail_->isNIL())
        return;

    // Save tail node and move tail to previous node
    Node *n = tail_;
    tail_ = tail_->getPrevious();

    // Update links: if new tail exists, set its next to NIL, otherwise set head to NIL
    if (!tail_->isNIL())
        tail_->setNext(&Node::NIL);
    else
        head_ = &Node::NIL;

    // Delete the old tail node and decrement size
    delete n;
    --size_;
}

// Inserts an element before the specified position (copy)
template <typename T>
typename DoublyLinkedList<T>::iterator
DoublyLinkedList<T>::insert(const_iterator pos, const T &value)
{
    // If position is end, append to back
    if (pos.node_->isNIL())
    {
        push_back(value);
        return iterator(tail_);
    }

    // Create new node and insert before current node
    Node *current = pos.node_;
    Node *n = new Node(value);

    current->prepend(n);

    // Update head if inserting at the beginning
    if (current == head_)
        head_ = n;

    ++size_;
    return iterator(n);
}

// Inserts an element before the specified position (move)
template <typename T>
typename DoublyLinkedList<T>::iterator
DoublyLinkedList<T>::insert(const_iterator pos, T &&value)
{
    // If position is end, append to back
    if (pos.node_->isNIL())
    {
        push_back(std::move(value));
        return iterator(tail_);
    }

    // Create new node and insert before current node
    Node *current = pos.node_;
    Node *n = new Node(std::move(value));

    current->prepend(n);

    // Update head if inserting at the beginning
    if (current == head_)
        head_ = n;

    ++size_;
    return iterator(n);
}

// Removes the element at the specified position
template <typename T>
typename DoublyLinkedList<T>::iterator
DoublyLinkedList<T>::erase(iterator pos)
{
    Node *current = pos.node_;
    // Return end iterator if position is NIL
    if (current->isNIL())
        return end();

    // Get previous and next nodes
    Node *prev = current->getPrevious();
    Node *next = current->getNext();

    // Update previous node's next pointer, or update head if at beginning
    if (!prev->isNIL())
        prev->setNext(next);
    else
        head_ = next;

    // Update next node's previous pointer, or update tail if at end
    if (!next->isNIL())
        next->setPrevious(prev);
    else
        tail_ = prev;

    // Save iterator to next node before deleting
    iterator ret(next);
    delete current;
    --size_;

    return ret;
}

// Removes all elements that satisfy the predicate
template <typename T>
template <typename Predicate>
int DoublyLinkedList<T>::removeIf(Predicate pred)
{
    int removed = 0;
    Node *cur = head_;

    // Iterate through all nodes
    while (!cur->isNIL())
    {
        // Save next node before potentially deleting current
        Node *nxt = cur->getNext();

        // Check if current node satisfies the predicate
        if (pred(cur->getValueRef()))
        {
            Node *prev = cur->getPrevious();

            // Update previous node's next pointer, or update head if at beginning
            if (!prev->isNIL())
                prev->setNext(cur->getNext());
            else
                head_ = cur->getNext();

            // Update next node's previous pointer, or update tail if at end
            if (!cur->getNext()->isNIL())
                cur->getNext()->setPrevious(prev);
            else
                tail_ = prev;

            // Delete node and update counters
            delete cur;
            --size_;
            ++removed;
        }

        // Move to next node
        cur = nxt;
    }

    return removed;
}

// Applies a function to each element in the list (non-const)
template <typename T>
template <typename Func>
void DoublyLinkedList<T>::forEach(Func func)
{
    Node *cur = head_;
    int elementCount = 0;

    // Iterate through all nodes and apply function
    while (!cur->isNIL())
    {
        func(cur->getValueRef());
        cur = cur->getNext();
        elementCount++;
    }

    if (ENABLE_ITERATOR_STDOUT && elementCount > 0)
        std::cout << "[Iterator] forEach() processed " << elementCount << " elements" << std::endl;
}

// Applies a function to each element in the list (const)
template <typename T>
template <typename Func>
void DoublyLinkedList<T>::forEach(Func func) const
{
    Node *cur = head_;

    // Iterate through all nodes and apply function
    while (!cur->isNIL())
    {
        func(cur->getValueRef());
        cur = cur->getNext();
    }
}

// Removes all elements from the list
template <typename T>
void DoublyLinkedList<T>::clear() noexcept
{
    Node *cur = head_;

    // Delete all nodes in the list
    while (!cur->isNIL())
    {
        Node *nxt = cur->getNext();
        delete cur;
        cur = nxt;
    }

    // Reset list to empty state
    head_ = tail_ = &Node::NIL;
    size_ = 0;
}

// Helper function to add a node to the front of the list
template <typename T>
void DoublyLinkedList<T>::push_front_node(Node *n)
{
    // Set new node's pointers
    n->setNext(head_);
    n->setPrevious(&Node::NIL);

    // Update head's previous pointer if list is not empty, otherwise set tail
    if (!head_->isNIL())
        head_->setPrevious(n);
    else
        tail_ = n;

    // Update head and increment size
    head_ = n;
    ++size_;
}

// Helper function to add a node to the back of the list
template <typename T>
void DoublyLinkedList<T>::push_back_node(Node *n)
{
    // Set new node's pointers
    n->setPrevious(tail_);
    n->setNext(&Node::NIL);

    // Update tail's next pointer if list is not empty, otherwise set head
    if (!tail_->isNIL())
        tail_->setNext(n);
    else
        head_ = n;

    // Update tail and increment size
    tail_ = n;
    ++size_;
}
