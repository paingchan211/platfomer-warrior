#pragma once

#include <iostream>
#include <stdexcept>
#include <utility>
#include "Constants.h"

// Template class for a singly linked list
template <typename T>
class SinglyLinkedList
{
private:
    // Inner struct representing a node in the list
    struct Node
    {
        T data;     // Data stored in the node
        Node *next; // Pointer to the next node
        Node(T value, Node *nextNode = nullptr) : data(value), next(nextNode) {}
    };

    Node *head; // Pointer to the first node
    Node *tail; // Pointer to the last node
    int count;  // Number of elements in the list

    void copyFrom(const SinglyLinkedList &other); // Helper to copy elements
    void swap(SinglyLinkedList &other) noexcept;  // Helper to swap two lists

public:
    SinglyLinkedList() : head(nullptr), tail(nullptr), count(0) {} // Default constructor

    SinglyLinkedList(const SinglyLinkedList &other) : head(nullptr), tail(nullptr), count(0)
    {
        copyFrom(other); // Copy constructor
    }

    SinglyLinkedList &operator=(const SinglyLinkedList &other)
    {
        if (this != &other)
        {
            SinglyLinkedList tmp(other); // Copy-and-swap idiom
            swap(tmp);
        }
        return *this;
    }

    // Move constructor
    SinglyLinkedList(SinglyLinkedList &&other) noexcept : head(other.head), tail(other.tail), count(other.count)
    {
        other.head = nullptr;
        other.tail = nullptr;
        other.count = 0;
    }

    // Move assignment operator
    SinglyLinkedList &operator=(SinglyLinkedList &&other) noexcept
    {
        if (this != &other)
        {
            clear(); // Free current resources
            head = other.head;
            tail = other.tail;
            count = other.count;
            other.head = nullptr;
            other.tail = nullptr;
            other.count = 0;
        }
        return *this;
    }

    // Destructor
    ~SinglyLinkedList()
    {
        clear();
    }

    // Add element to the end of the list
    void pushBack(T value)
    {
        Node *newNode = new Node(value);
        if (tail == nullptr)
        {
            head = tail = newNode;
        }
        else
        {
            tail->next = newNode;
            tail = newNode;
        }
        ++count;
    }

    // Add element to the front of the list
    void pushFront(T value)
    {
        Node *newNode = new Node(value, head);
        head = newNode;
        if (tail == nullptr)
        {
            tail = newNode;
        }
        ++count;
    }

    // Remove element from the front of the list
    void popFront()
    {
        if (head == nullptr)
            throw std::runtime_error("Cannot pop from empty list");

        Node *toDelete = head;
        head = head->next;
        if (head == nullptr)
            tail = nullptr;
        delete toDelete;
        --count;
    }

    // Remove nodes based on a condition
    template <typename Predicate>
    int removeIf(Predicate predicate)
    {
        int removed = 0;
        Node *current = head;
        Node *previous = nullptr;
        while (current != nullptr)
        {
            if (predicate(current->data))
            {
                Node *toDelete = current;
                if (previous == nullptr)
                {
                    head = current->next;
                    current = head;
                    if (head == nullptr)
                        tail = nullptr;
                }
                else
                {
                    previous->next = current->next;
                    current = current->next;
                    if (toDelete == tail)
                        tail = previous;
                }
                delete toDelete;
                --count;
                ++removed;
            }
            else
            {
                previous = current;
                current = current->next;
            }
        }
        return removed;
    }

    // Apply function to each element
    template <typename Func>
    void forEach(Func func)
    {
        Node *current = head;
        while (current != nullptr)
        {
            func(current->data);
            current = current->next;
        }
    }

    // Const version of forEach
    template <typename Func>
    void forEach(Func func) const
    {
        Node *current = head;
        while (current != nullptr)
        {
            func(current->data);
            current = current->next;
        }
    }

    // Clear all elements from the list
    void clear()
    {
        Node *current = head;
        while (current != nullptr)
        {
            Node *next = current->next;
            delete current;
            current = next;
        }
        head = tail = nullptr;
        count = 0;
    }

    // Check if list is empty
    bool isEmpty() const { return head == nullptr; }

    // Return number of elements
    int size() const { return count; }

    // Get front element
    T front() const
    {
        if (head == nullptr)
            throw std::runtime_error("Cannot get front of empty list");
        return head->data;
    }

    // Get last element
    T back() const
    {
        if (tail == nullptr)
            throw std::runtime_error("Cannot get back of empty list");
        return tail->data;
    }
};

// Swap helper function
template <typename T>
void SinglyLinkedList<T>::swap(SinglyLinkedList<T> &other) noexcept
{
    std::swap(this->head, other.head);
    std::swap(this->tail, other.tail);
    std::swap(this->count, other.count);
}

// Copy helper function
template <typename T>
void SinglyLinkedList<T>::copyFrom(const SinglyLinkedList<T> &other)
{
    Node *current = other.head;
    while (current != nullptr)
    {
        pushBack(current->data);
        current = current->next;
    }
}