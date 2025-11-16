#pragma once

#include <iostream>  // For optional debugging/logging
#include <stdexcept> // For exceptions
#include "Constants.h"

// Template class implementing a basic stack using a singly linked list.
template <typename T>
class Stack
{
private:
    // Node structure holding one stack element and a pointer to the next node.
    struct Node
    {
        T data;     // Stored value
        Node *next; // Pointer to the node below this one

        Node(const T &value, Node *nextNode = nullptr); // Copy constructor
        Node(T &&value, Node *nextNode = nullptr);      // Move constructor
    };

    Node *topNode; // Pointer to the top node in the stack
    int count;     // Number of stored elements

public:
    Stack();  // Default constructor creates an empty stack
    ~Stack(); // Destructor clears all nodes

    Stack(const Stack &) = delete;            // Disable copying
    Stack &operator=(const Stack &) = delete; // Disable copy assignment

    Stack(Stack &&other) noexcept;            // Move constructor
    Stack &operator=(Stack &&other) noexcept; // Move assignment

    void push(const T &value); // Push an element by copy
    void push(T &&value);      // Push an element by move

    void pop(); // Remove top element, throws if empty

    T &top();             // Non-const reference to top element
    const T &top() const; // Const reference to top element

    T &peek(int level);             // Peek element deeper in the stack
    const T &peek(int level) const; // Const version of peek

    bool isEmpty() const; // True if stack is empty
    int size() const;     // Number of stored nodes
    void clear();         // Delete all nodes
};

// -------------------- Implementation --------------------

// Node copy constructor
template <typename T>
Stack<T>::Node::Node(const T &value, Node *nextNode)
    : data(value), next(nextNode) // Store a copy of the value and link node
{
}

// Node move constructor
template <typename T>
Stack<T>::Node::Node(T &&value, Node *nextNode)
    : data(std::move(value)), next(nextNode) // Move payload and link
{
}

// Default constructor
template <typename T>
Stack<T>::Stack()
    : topNode(nullptr), count(0) // Start with empty stack
{
}

// Destructor
template <typename T>
Stack<T>::~Stack()
{
    clear(); // Release all nodes
}

// Move constructor: steal node chain
template <typename T>
Stack<T>::Stack(Stack &&other) noexcept
    : topNode(other.topNode), count(other.count)
{
    other.topNode = nullptr; // Reset donor
    other.count = 0;
}

// Move assignment operator: clear then steal
template <typename T>
Stack<T> &Stack<T>::operator=(Stack &&other) noexcept
{
    if (this != &other)
    {
        clear(); // Remove current nodes
        topNode = other.topNode;
        count = other.count;

        other.topNode = nullptr; // Leave other empty
        other.count = 0;
    }
    return *this;
}

// Push by copying value
template <typename T>
void Stack<T>::push(const T &value)
{
    Node *newNode = new Node(value, topNode); // New node becomes top
    topNode = newNode;
    ++count;
}

// Push by moving value
template <typename T>
void Stack<T>::push(T &&value)
{
    Node *newNode = new Node(std::move(value), topNode);
    topNode = newNode;
    ++count;
}

// Pop removes top element
template <typename T>
void Stack<T>::pop()
{
    if (isEmpty())
        throw std::runtime_error("Stack::pop() called on empty stack");

    Node *oldTop = topNode;  // Save node to delete
    topNode = topNode->next; // Move down one level
    delete oldTop;           // Free memory
    --count;
}

// Access non-const top element
template <typename T>
T &Stack<T>::top()
{
    if (isEmpty())
        throw std::runtime_error("Stack::top() called on empty stack");

    return topNode->data;
}

// Access const top element
template <typename T>
const T &Stack<T>::top() const
{
    if (isEmpty())
        throw std::runtime_error("Stack::top() called on empty stack");

    return topNode->data;
}

// Peek deeper into stack (0 = top)
template <typename T>
T &Stack<T>::peek(int level)
{
    if (level < 0 || level >= count)
        throw std::out_of_range("Stack::peek() level out of range");

    Node *current = topNode;
    for (int i = 0; i < level; ++i)
        current = current->next;

    return current->data;
}

// Const peek
template <typename T>
const T &Stack<T>::peek(int level) const
{
    if (level < 0 || level >= count)
        throw std::out_of_range("Stack::peek() level out of range");

    Node *current = topNode;
    for (int i = 0; i < level; ++i)
        current = current->next;

    return current->data;
}

// Check if empty
template <typename T>
bool Stack<T>::isEmpty() const
{
    return topNode == nullptr;
}

// Number of elements
template <typename T>
int Stack<T>::size() const
{
    return count;
}

// Clear all nodes
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
