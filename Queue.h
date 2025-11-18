#pragma once

#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>
#include "FloatingText.h"

// Default logger that does nothing; specialized types can override
template <typename T>
struct QueueDebugLogger
{
    static void enqueued(const T &) {}
    static void dequeued(const T &) {}
};

// Specialization for floating text queue entries
template <>
struct QueueDebugLogger<std::unique_ptr<FloatingText>>
{
    static void enqueued(const std::unique_ptr<FloatingText> &value)
    {
        if (value)
        {
            std::cout << "[Queue] '" << value->getDisplayString() << "' enqueued" << std::endl;
        }
    }

    static void dequeued(const std::unique_ptr<FloatingText> &value)
    {
        if (value)
        {
            std::cout << "[Queue] '" << value->getDisplayString() << "' dequeued" << std::endl;
        }
    }
};

// Simple templated linked-list queue implementation.
// Supports enqueue/dequeue, iteration (forEach), and conditional removal (removeIf).
template <typename T>
class Queue
{
private:
    // Node struct represents one queue element
    struct Node
    {
        T data;     // Stored data in this node
        Node *next; // Pointer to next node in queue

        Node(const T &value); // Constructor for copying value
        Node(T &&value);      // Constructor for moving value
    };

    Node *frontNode; // Pointer to first element in the queue
    Node *rearNode;  // Pointer to last element in the queue
    int count;       // Number of elements currently stored

public:
    Queue();  // Creates an empty queue
    ~Queue(); // Frees all nodes to avoid memory leaks

    void enqueue(const T &value); // Add element to back (copy)
    void enqueue(T &&value);      // Add element to back (move)

    void dequeue(); // Remove the front element, throws if empty

    T &front();             // Get reference to front element (non-const)
    const T &front() const; // Get reference to front element (const)

    bool isEmpty() const; // Returns true if queue is empty
    int size() const;     // Returns the number of stored elements
    void clear();         // Delete all nodes

    template <typename Func>
    void forEach(Func func); // Apply function to each element

    template <typename Predicate>
    void removeIf(Predicate pred); // Remove all elements matching predicate
};

// ---------- Implementation ----------

// Copy-construct node from value and set next to nullptr.
template <typename T>
Queue<T>::Node::Node(const T &value) : data(value), next(nullptr) {}

// Move-construct node from value and set next to nullptr.
template <typename T>
Queue<T>::Node::Node(T &&value) : data(std::move(value)), next(nullptr) {}

// Initialize an empty queue with null pointers and count = 0.
template <typename T>
Queue<T>::Queue() : frontNode(nullptr), rearNode(nullptr), count(0) {}

// Destructor deletes all nodes to prevent memory leaks.
template <typename T>
Queue<T>::~Queue()
{
    clear(); // Removes every node safely
}

// Add element by copying and attach it at the end of the queue.
template <typename T>
void Queue<T>::enqueue(const T &value)
{
    Node *newNode = new Node(value); // Allocate a new node

    if (isEmpty())
    {
        // If queue is empty, front and rear become the new node
        frontNode = rearNode = newNode;
    }
    else
    {
        // Otherwise append to rear
        rearNode->next = newNode;
        rearNode = newNode;
    }

    count++; // Increase size
    QueueDebugLogger<T>::enqueued(newNode->data);
}

// Add element by move semantics (avoids copying large objects).
template <typename T>
void Queue<T>::enqueue(T &&value)
{
    Node *newNode = new Node(std::move(value));

    if (isEmpty())
    {
        frontNode = rearNode = newNode;
    }
    else
    {
        rearNode->next = newNode;
        rearNode = newNode;
    }

    count++;
    QueueDebugLogger<T>::enqueued(newNode->data);
}

// Remove the element at the front of the queue. Throws if the queue is empty.
template <typename T>
void Queue<T>::dequeue()
{
    if (isEmpty())
    {
        throw std::runtime_error("Queue is empty - cannot dequeue");
    }

    Node *temp = frontNode;      // Store node to delete
    frontNode = frontNode->next; // Move front pointer forward

    if (frontNode == nullptr)
    {
        rearNode = nullptr; // Queue is now empty
    }

    QueueDebugLogger<T>::dequeued(temp->data);
    delete temp; // Free old front node
    count--;     // Decrease size
}

// Returns reference to the front data (modifiable).
template <typename T>
T &Queue<T>::front()
{
    if (isEmpty())
    {
        throw std::runtime_error("Queue is empty - cannot peek front");
    }
    return frontNode->data;
}

// Returns reference to front data (read-only).
template <typename T>
const T &Queue<T>::front() const
{
    if (isEmpty())
    {
        throw std::runtime_error("Queue is empty - cannot peek front");
    }
    return frontNode->data;
}

// Returns true if queue has no elements.
template <typename T>
bool Queue<T>::isEmpty() const
{
    return frontNode == nullptr;
}

// Returns number of nodes stored.
template <typename T>
int Queue<T>::size() const
{
    return count;
}

// Removes every node in the queue.
template <typename T>
void Queue<T>::clear()
{
    while (!isEmpty())
    {
        dequeue(); // Reuse dequeue() to delete nodes safely
    }
}

// Execute function 'func' on every node's data.
template <typename T>
template <typename Func>
void Queue<T>::forEach(Func func)
{
    Node *current = frontNode; // Start at front
    while (current != nullptr)
    {
        func(current->data);     // Apply function to current element
        current = current->next; // Move to next
    }
}

// Remove all elements where predicate(pred(data)) returns true.
template <typename T>
template <typename Predicate>
void Queue<T>::removeIf(Predicate pred)
{
    Node *current = frontNode;
    Node *previous = nullptr;

    while (current != nullptr)
    {
        Node *next = current->next; // Save next pointer before modifications

        if (pred(current->data))
        {
            // Node must be removed
            if (previous == nullptr)
            {
                // Removing the first node
                frontNode = next;

                if (frontNode == nullptr)
                {
                    rearNode = nullptr; // Queue now empty
                }
            }
            else
            {
                // Removing node in the middle or end
                previous->next = next;

                if (current == rearNode)
                {
                    rearNode = previous; // Update rear if needed
                }
            }

            delete current; // Free memory
            count--;        // Adjust size
        }
        else
        {
            // Keep node, move previous pointer
            previous = current;
        }

        current = next; // Move forward
    }
}
