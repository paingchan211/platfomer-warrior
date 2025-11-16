#pragma once

#include <iostream>  // For potential debugging/logging hooks
#include <stdexcept> // For consistent exception reporting
#include "Constants.h"

// Template class implementing a basic stack using a singly linked list.
// Provides push/pop, indexed peek, top access, clear, and move semantics with
// predictable O(1) operations for the common stack behaviors.
template <typename T>
class Stack
{
private:
    // Node structure for stack elements
    struct Node
    {
        T data;                                                                               // Payload stored in this node
        Node *next;                                                                           // Pointer to the node below the current one
        Node(const T &value, Node *nextNode = nullptr) : data(value), next(nextNode) {}       // Copy constructor retains caller data
        Node(T &&value, Node *nextNode = nullptr) : data(std::move(value)), next(nextNode) {} // Move constructor for efficiency on temporaries
    };

    Node *topNode; // Pointer to the stack's head (most recently pushed element)
    int count;     // Number of elements currently stored (helps implement O(1) size/peek checks)

public:
    Stack();                                  // Default constructor initializes an empty stack
    ~Stack();                                 // Destructor clears all nodes to avoid leaks
    Stack(const Stack &) = delete;            // Disable copy constructor
    Stack &operator=(const Stack &) = delete; // Disable copy assignment

    Stack(Stack &&other) noexcept;            // Move constructor transfers ownership
    Stack &operator=(Stack &&other) noexcept; // Move assignment steals nodes

    void push(const T &value); // Push element by copying value
    void push(T &&value);      // Push element by moving value

    void pop(); // Pop top element, throwing if empty

    T &top();             // Non-const access to top element
    const T &top() const; // Const access to top element

    T &peek(int level);             // Peek element at specific level from top
    const T &peek(int level) const; // Const peek

    bool isEmpty() const; // Check if stack is empty
    int size() const;     // Number of stored elements
    void clear();         // Remove all nodes
};

// ---------- Implementation ----------

// Default constructor initializes an empty stack (null head, zero count).
template <typename T>
Stack<T>::Stack() : topNode(nullptr), count(0) {}

// Destructor clears all nodes to avoid leaks.
template <typename T>
Stack<T>::~Stack()
{
    clear();
}

// Move constructor transfers ownership of the node chain from another stack.
template <typename T>
Stack<T>::Stack(Stack &&other) noexcept : topNode(other.topNode), count(other.count)
{
    other.topNode = nullptr;
    other.count = 0;
}

// Move assignment operator clears current contents and steals nodes.
template <typename T>
Stack<T> &Stack<T>::operator=(Stack &&other) noexcept
{
    if (this != &other)
    {
        clear(); // delete current nodes before stealing
        topNode = other.topNode;
        count = other.count;
        other.topNode = nullptr;
        other.count = 0;
    }
    return *this;
}

// Push element by copying value to the top of the stack.
// The new node becomes the new head and the previous head becomes its next pointer.
template <typename T>
void Stack<T>::push(const T &value)
{
    Node *newNode = new Node(value, topNode); // new node points to previous head
    topNode = newNode;
    ++count;
}

// Push element by moving value to the top of the stack.
// Useful when the caller owns a temporary and wants to avoid a copy.
template <typename T>
void Stack<T>::push(T &&value)
{
    Node *newNode = new Node(std::move(value), topNode); // move payload to new head
    topNode = newNode;
    ++count;
}

// Pop top element from stack, throwing if the stack is empty.
// The removed node is deleted and the next node becomes the new head.
template <typename T>
void Stack<T>::pop()
{
    if (isEmpty())
    {
        throw std::runtime_error("Stack::pop() called on empty stack");
    }
    Node *oldTop = topNode;  // keep pointer so we can delete after relinking
    topNode = topNode->next; // drop one level down the stack
    delete oldTop;           // free node memory
    --count;
}

// Access top element (non-const) and throw if empty.
// Returned reference aliases the stored data; caller should avoid holding it past pop().
template <typename T>
T &Stack<T>::top()
{
    if (isEmpty())
    {
        throw std::runtime_error("Stack::top() called on empty stack");
    }
    return topNode->data;
}

// Access top element (const) and throw if empty.
// Allows const stacks to inspect the head element without mutation.
template <typename T>
const T &Stack<T>::top() const
{
    if (isEmpty())
    {
        throw std::runtime_error("Stack::top() called on empty stack");
    }
    return topNode->data;
}

// Peek element at specific level from top (0 == top), throwing for invalid level.
// Useful for debugging the stack or rendering state without popping.
template <typename T>
T &Stack<T>::peek(int level)
{
    if (level < 0 || level >= count)
    {
        throw std::out_of_range("Stack::peek() level out of range");
    }
    Node *current = topNode;
    for (int i = 0; i < level; ++i) // walk down until we reach desired depth
    {
        current = current->next;
    }
    return current->data;
}

// Const version of peek with the same bounds checking.
template <typename T>
const T &Stack<T>::peek(int level) const
{
    if (level < 0 || level >= count)
    {
        throw std::out_of_range("Stack::peek() level out of range");
    }
    Node *current = topNode;
    for (int i = 0; i < level; ++i) // walk down until we reach desired depth
    {
        current = current->next;
    }
    return current->data;
}

// Check if stack is empty.
// Equivalent to checking count == 0 but avoids depending on an int comparison.
template <typename T>
bool Stack<T>::isEmpty() const
{
    return topNode == nullptr;
}

// Return number of elements currently stored.
template <typename T>
int Stack<T>::size() const
{
    return count;
}

// Clear all elements in the stack, deleting allocated nodes.
// Safe to call even if the stack is already empty.
template <typename T>
void Stack<T>::clear()
{
    while (!isEmpty())
    {
        Node *temp = topNode;
        topNode = topNode->next;
        delete temp;
    }
    count = 0;
}
