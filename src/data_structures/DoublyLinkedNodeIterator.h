#pragma once

// Forward declaration of node class so the iterator can refer to it.
template <typename DataType>
class DoublyLinkedNode;

// Forward declaration of list class so iterator can be a friend.
template <typename T>
class DoublyLinkedList;

// Iterator for DoublyLinkedNode (non-const version only).
template <typename T>
class DoublyLinkedNodeIterator
{
    using Node = DoublyLinkedNode<T>; // Alias for the node type
    friend class DoublyLinkedList<T>; // List needs access to internal pointer

public:
    struct iterator_category
    {
    }; // Minimal iterator category tag
    using difference_type = long; // Difference type for iterator arithmetic
    using pointer = T *;          // Pointer to stored data
    using reference = T &;        // Reference to stored data

    DoublyLinkedNodeIterator();                 // Default constructor → points to NIL sentinel
    explicit DoublyLinkedNodeIterator(Node *n); // Construct iterator from node pointer

    DoublyLinkedNodeIterator(const DoublyLinkedNodeIterator &other) = default; // Copy OK
    DoublyLinkedNodeIterator &operator=(const DoublyLinkedNodeIterator &other) = default;

    reference operator*() const; // Dereference → return value reference
    pointer operator->() const;  // Arrow operator → return pointer to value

    DoublyLinkedNodeIterator &operator++();   // Prefix ++ → move to next node
    DoublyLinkedNodeIterator operator++(int); // Postfix ++

    DoublyLinkedNodeIterator &operator--();   // Prefix -- → move to previous node
    DoublyLinkedNodeIterator operator--(int); // Postfix --

    bool operator==(const DoublyLinkedNodeIterator &other) const; // Compare equality
    bool operator!=(const DoublyLinkedNodeIterator &other) const; // Compare inequality

private:
    Node *node; // Pointer to current node in the list
};

// ========================== IMPLEMENTATION ================================

// Default constructor → point iterator at the NIL sentinel node.
template <typename T>
DoublyLinkedNodeIterator<T>::DoublyLinkedNodeIterator()
    : node(&Node::NIL) // Node::NIL is the sentinel representing "end"
{
}

// Constructor → wrap actual node pointer.
template <typename T>
DoublyLinkedNodeIterator<T>::DoublyLinkedNodeIterator(Node *n)
    : node(n)
{
}

// Dereference operator → returns reference to the node's stored value.
template <typename T>
typename DoublyLinkedNodeIterator<T>::reference
DoublyLinkedNodeIterator<T>::operator*() const
{
    // getValueRef() returns T& stored in the node
    return node->getValueRef();
}

// Arrow operator → returns pointer to the stored value.
template <typename T>
typename DoublyLinkedNodeIterator<T>::pointer
DoublyLinkedNodeIterator<T>::operator->() const
{
    // Address of getValueRef() gives pointer to stored value
    return &node->getValueRef();
}

// Prefix increment → move iterator to next node.
template <typename T>
DoublyLinkedNodeIterator<T> &
DoublyLinkedNodeIterator<T>::operator++()
{
    node = node->getNext(); // Step forward
    return *this;
}

// Postfix increment → return old iterator, then increment.
template <typename T>
DoublyLinkedNodeIterator<T>
DoublyLinkedNodeIterator<T>::operator++(int)
{
    DoublyLinkedNodeIterator tmp(*this); // Save old state
    ++(*this);                           // Move to next
    return tmp;                          // Return previous iterator
}

// Prefix decrement → move iterator to previous node.
template <typename T>
DoublyLinkedNodeIterator<T> &
DoublyLinkedNodeIterator<T>::operator--()
{
    node = node->getPrevious(); // Step backward
    return *this;
}

// Postfix decrement → return old iterator, then decrement.
template <typename T>
DoublyLinkedNodeIterator<T> DoublyLinkedNodeIterator<T>::operator--(int)
{
    DoublyLinkedNodeIterator tmp(*this); // Save old state
    --(*this);                           // Move to previous
    return tmp;                          // Return previous iterator
}

// Equality comparison → true if both iterators point to same node.
template <typename T>
bool DoublyLinkedNodeIterator<T>::operator==(const DoublyLinkedNodeIterator &other) const
{
    return node == other.node;
}

// Inequality comparison.
template <typename T>
bool DoublyLinkedNodeIterator<T>::operator!=(const DoublyLinkedNodeIterator &other) const
{
    return node != other.node;
}