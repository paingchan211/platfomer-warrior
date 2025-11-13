#pragma once

#include <iostream>
#include <stdexcept>
#include "Constants.h"

// Template class implementing a basic stack using a singly linked list
template <typename T>
class Stack
{
private:
    // Node structure for stack elements
    struct Node
    {
        T data;                                                                               // Data stored in the node
        Node *next;                                                                           // Pointer to next node
        Node(const T &value, Node *nextNode = nullptr) : data(value), next(nextNode) {}       // Copy constructor
        Node(T &&value, Node *nextNode = nullptr) : data(std::move(value)), next(nextNode) {} // Move constructor
    };

    Node *topNode; // Pointer to top of the stack
    int count;     // Number of elements in the stack

public:
    // Default constructor
    Stack() : topNode(nullptr), count(0) {}

    // Destructor clears all nodes
    ~Stack()
    {
        clear();
    }

    // Disable copy constructor and assignment
    Stack(const Stack &) = delete;
    Stack &operator=(const Stack &) = delete;

    // Move constructor
    Stack(Stack &&other) noexcept : topNode(other.topNode), count(other.count)
    {
        other.topNode = nullptr;
        other.count = 0;
    }

    // Move assignment operator
    Stack &operator=(Stack &&other) noexcept
    {
        if (this != &other)
        {
            clear();
            topNode = other.topNode;
            count = other.count;
            other.topNode = nullptr;
            other.count = 0;
        }
        return *this;
    }

    // Push element (copy version)
    void push(const T &value)
    {
        Node *newNode = new Node(value, topNode);
        topNode = newNode;
        ++count;
    }

    // Push element (move version)
    void push(T &&value)
    {
        Node *newNode = new Node(std::move(value), topNode);
        topNode = newNode;
        ++count;
    }

    // Pop top element from stack
    void pop()
    {
        if (isEmpty())
        {
            throw std::runtime_error("Stack::pop() called on empty stack");
        }
        Node *oldTop = topNode;
        topNode = topNode->next;
        delete oldTop;
        --count;
    }

    // Access top element (non-const)
    T &top()
    {
        if (isEmpty())
        {
            throw std::runtime_error("Stack::top() called on empty stack");
        }
        return topNode->data;
    }

    // Access top element (const)
    const T &top() const
    {
        if (isEmpty())
        {
            throw std::runtime_error("Stack::top() called on empty stack");
        }
        return topNode->data;
    }

    // Peek element at specific level from top
    T &peek(int level)
    {
        if (level < 0 || level >= count)
        {
            throw std::out_of_range("Stack::peek() level out of range");
        }
        Node *current = topNode;
        for (int i = 0; i < level; ++i)
        {
            current = current->next;
        }
        return current->data;
    }

    // Const version of peek
    const T &peek(int level) const
    {
        if (level < 0 || level >= count)
        {
            throw std::out_of_range("Stack::peek() level out of range");
        }
        Node *current = topNode;
        for (int i = 0; i < level; ++i)
        {
            current = current->next;
        }
        return current->data;
    }

    // Check if stack is empty
    bool isEmpty() const
    {
        return topNode == nullptr;
    }

    // Return number of elements
    int size() const
    {
        return count;
    }

    // Clear all elements in the stack
    void clear()
    {
        while (!isEmpty())
        {
            Node *temp = topNode;
            topNode = topNode->next;
            delete temp;
        }
        count = 0;
    }
};