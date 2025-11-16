#pragma once

#include <iostream>
#include <stdexcept>
#include <utility>
#include "Constants.h"

// Template class for a singly linked list
// Used for storing combat log entries in chronological order
template <typename T>
class SinglyLinkedList
{
private:
    // Inner struct representing a node in the list
    // Each node contains data and a pointer to the next node
    struct Node
    {
        T data;     // Data stored in the node (e.g., combat log message string)
        Node *next; // Pointer to the next node in the sequence

        // Node constructor: initializes data and optionally links to next node
        Node(T value, Node *nextNode = nullptr) : data(value), next(nextNode) {}
    };

    Node *head; // Pointer to the first node (oldest combat log entry)
    Node *tail; // Pointer to the last node (newest combat log entry)
    int count;  // Number of elements in the list (total log entries)

    // Helper function to copy all elements from another list
    void copyFrom(const SinglyLinkedList &other);

    // Helper function to swap contents of two lists (used in copy-and-swap idiom)
    void swap(SinglyLinkedList &other) noexcept;

public:
    // Default constructor: creates an empty list
    SinglyLinkedList() : head(nullptr), tail(nullptr), count(0) {}

    // Copy constructor: creates a deep copy of another list
    SinglyLinkedList(const SinglyLinkedList &other) : head(nullptr), tail(nullptr), count(0)
    {
        copyFrom(other);
    }

    // Copy assignment operator: uses copy-and-swap idiom for exception safety
    SinglyLinkedList &operator=(const SinglyLinkedList &other)
    {
        if (this != &other)
        {
            SinglyLinkedList tmp(other); // Create temporary copy
            swap(tmp);                   // Swap with temporary (exception-safe)
        }
        return *this;
    }

    // Move constructor: transfers ownership of resources from another list
    SinglyLinkedList(SinglyLinkedList &&other) noexcept
        : head(other.head), tail(other.tail), count(other.count)
    {
        // Leave the source list in a valid empty state
        other.head = nullptr;
        other.tail = nullptr;
        other.count = 0;
    }

    // Move assignment operator: transfers ownership with cleanup of current resources
    SinglyLinkedList &operator=(SinglyLinkedList &&other) noexcept
    {
        if (this != &other)
        {
            clear(); // Free current resources first

            // Transfer ownership from source
            head = other.head;
            tail = other.tail;
            count = other.count;

            // Leave source in valid empty state
            other.head = nullptr;
            other.tail = nullptr;
            other.count = 0;
        }
        return *this;
    }

    // Destructor: cleans up all nodes to prevent memory leaks
    ~SinglyLinkedList()
    {
        clear();
    }

    // Add element to the end of the list (newest combat log entry)
    // Time Complexity: O(1) - constant time due to tail pointer
    void pushBack(T value)
    {
        Node *newNode = new Node(value); // Allocate new node with the value

        if (tail == nullptr) // List is empty
        {
            head = tail = newNode; // New node is both head and tail
        }
        else // List has existing elements
        {
            tail->next = newNode; // Link current tail to new node
            tail = newNode;       // Update tail to point to new node
        }
        ++count; // Increment total element count
    }

    // Add element to the front of the list
    // Time Complexity: O(1) - constant time
    void pushFront(T value)
    {
        Node *newNode = new Node(value, head); // Create node pointing to current head
        head = newNode;                        // Update head to new node

        if (tail == nullptr) // List was empty
        {
            tail = newNode; // New node is also the tail
        }
        ++count; // Increment total element count
    }

    // Remove element from the front of the list (remove oldest combat log entry)
    // Time Complexity: O(1) - constant time
    // Used when combat log exceeds MAX_COMBAT_LOG_ENTRIES capacity
    void popFront()
    {
        if (head == nullptr) // Cannot remove from empty list
            throw std::runtime_error("Cannot pop from empty list");

        Node *toDelete = head; // Save pointer to node being deleted
        head = head->next;     // Move head to next node

        if (head == nullptr) // List is now empty
            tail = nullptr;  // Update tail to nullptr as well

        delete toDelete; // Free memory of removed node
        --count;         // Decrement element count
    }

    // Remove nodes based on a condition (predicate function)
    // Used for deleting specific combat log entries by index
    // Time Complexity: O(n) - must traverse list to find matching nodes
    template <typename Predicate>
    int removeIf(Predicate predicate)
    {
        int removed = 0;          // Counter for removed nodes
        Node *current = head;     // Start at the beginning
        Node *previous = nullptr; // Track previous node for pointer updates

        while (current != nullptr)
        {
            if (predicate(current->data)) // Test if node should be removed
            {
                Node *toDelete = current; // Save pointer to node being deleted

                if (previous == nullptr) // Removing the head node
                {
                    head = current->next; // Update head to next node
                    current = head;       // Continue iteration from new head

                    if (head == nullptr) // List is now empty
                        tail = nullptr;  // Update tail as well
                }
                else // Removing a middle or tail node
                {
                    previous->next = current->next; // Bypass the node being deleted
                    current = current->next;        // Continue iteration

                    if (toDelete == tail) // Removed node was the tail
                        tail = previous;  // Update tail to previous node
                }

                delete toDelete; // Free memory of removed node
                --count;         // Decrement element count
                ++removed;       // Increment removed counter
            }
            else // Node doesn't match predicate, keep it
            {
                previous = current;      // Update previous pointer
                current = current->next; // Move to next node
            }
        }
        return removed; // Return count of removed nodes
    }

    // Apply function to each element in the list
    // Used for rendering combat log entries to screen
    // Time Complexity: O(n) - must visit each node
    template <typename Func>
    void forEach(Func func)
    {
        Node *current = head; // Start at the beginning

        while (current != nullptr)
        {
            func(current->data);     // Apply function to current node's data
            current = current->next; // Move to next node
        }
    }

    // Const version of forEach (for read-only access)
    template <typename Func>
    void forEach(Func func) const
    {
        Node *current = head; // Start at the beginning

        while (current != nullptr)
        {
            func(current->data);     // Apply function to current node's data
            current = current->next; // Move to next node
        }
    }

    // Clear all elements from the list
    // Time Complexity: O(n) - must delete each node
    void clear()
    {
        Node *current = head; // Start at the beginning

        while (current != nullptr)
        {
            Node *next = current->next; // Save next pointer before deleting
            delete current;             // Free memory of current node
            current = next;             // Move to next node
        }

        // Reset list to empty state
        head = tail = nullptr;
        count = 0;
    }

    // Check if list is empty
    // Time Complexity: O(1)
    bool isEmpty() const { return head == nullptr; }

    // Return number of elements in the list
    // Time Complexity: O(1)
    int size() const { return count; }

    // Get front element (oldest combat log entry)
    // Time Complexity: O(1)
    T front() const
    {
        if (head == nullptr)
            throw std::runtime_error("Cannot get front of empty list");
        return head->data;
    }

    // Get last element (newest combat log entry)
    // Time Complexity: O(1)
    T back() const
    {
        if (tail == nullptr)
            throw std::runtime_error("Cannot get back of empty list");
        return tail->data;
    }
};

// Swap helper function implementation
// Exchanges all member variables between two lists
template <typename T>
void SinglyLinkedList<T>::swap(SinglyLinkedList<T> &other) noexcept
{
    std::swap(this->head, other.head);   // Swap head pointers
    std::swap(this->tail, other.tail);   // Swap tail pointers
    std::swap(this->count, other.count); // Swap element counts
}

// Copy helper function implementation
// Creates a deep copy of all nodes from another list
template <typename T>
void SinglyLinkedList<T>::copyFrom(const SinglyLinkedList<T> &other)
{
    Node *current = other.head; // Start at beginning of source list

    while (current != nullptr)
    {
        pushBack(current->data); // Add each element to this list
        current = current->next; // Move to next node in source
    }
}