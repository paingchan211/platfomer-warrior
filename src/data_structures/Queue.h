#pragma once

#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>
#include "FloatingText.h"

// Debug logger template (default: does nothing)
template <typename T>
struct QueueDebugLogger
{
    // Called when a value is enqueued (default: no output)
    static void enqueued(const T &);

    // Called when a value is dequeued (default: no output)
    static void dequeued(const T &);
};

// Specialization for unique_ptr<FloatingText> to print readable logs
template <>
struct QueueDebugLogger<std::unique_ptr<FloatingText>>
{
    // Log enqueue event for floating text
    static void enqueued(const std::unique_ptr<FloatingText> &value);

    // Log dequeue event for floating text
    static void dequeued(const std::unique_ptr<FloatingText> &value);
};

template <typename T>
class Queue
{
private:
    // Node of the singly linked list
    struct Node
    {
        T data;     // Stored value
        Node *next; // Pointer to next node

        // Construct a node by copying the value
        Node(const T &value);

        // Construct a node by moving the value
        Node(T &&value);
    };

    Node *frontNode; // First node (front of queue)
    Node *rearNode;  // Last node (back of queue)
    int count;       // Number of stored elements

public:
    // Create an empty queue
    Queue();

    // Destroy all nodes and free memory
    ~Queue();

    // Add a new element to the back (move semantics)
    void enqueue(T &&value);

    // Remove the element at the front
    void dequeue();

    // Access the element at the front (throws if empty)
    T &front();

    // Check if queue contains no elements
    bool isEmpty() const;

    // Number of stored elements
    int size() const;

    // Apply a callback function to every element (front → back)
    template <typename Func>
    void forEach(Func func);
};

// Default logger: no action
template <typename T>
void QueueDebugLogger<T>::enqueued(const T &) {}

template <typename T>
void QueueDebugLogger<T>::dequeued(const T &) {}

// Logging for unique_ptr<FloatingText>
inline void QueueDebugLogger<std::unique_ptr<FloatingText>>::enqueued(
    const std::unique_ptr<FloatingText> &value)
{
    // Only output if enabled and pointer exists
    if (value && ENABLE_QUEUE_STDOUT)
        std::cout << "[Queue] '" << value->getDisplayString() << "' enqueued\n";
}

inline void QueueDebugLogger<std::unique_ptr<FloatingText>>::dequeued(
    const std::unique_ptr<FloatingText> &value)
{
    // Log removal before node destruction
    if (value && ENABLE_QUEUE_STDOUT)
        std::cout << "[Queue] '" << value->getDisplayString() << "' dequeued\n";
}

// Copy constructor for Node
template <typename T>
Queue<T>::Node::Node(const T &value)
    : data(value), next(nullptr)
{
    // Copy the value into the new node
}

// Move constructor for Node
template <typename T>
Queue<T>::Node::Node(T &&value)
    : data(std::move(value)), next(nullptr)
{
    // Move value for efficiency
}

// Queue constructor
template <typename T>
Queue<T>::Queue()
    : frontNode(nullptr), rearNode(nullptr), count(0)
{
    // Initially empty queue with null pointers
}

// Queue destructor
template <typename T>
Queue<T>::~Queue()
{
    // Remove all nodes
    while (!isEmpty())
        dequeue();
}

// Add value to the back of the queue
template <typename T>
void Queue<T>::enqueue(T &&value)
{
    Node *newNode = new Node(std::move(value)); // Allocate new node

    if (isEmpty())
    {
        // If queue is empty, front and rear both point to new node
        frontNode = rearNode = newNode;
    }
    else
    {
        // Attach to the current last node
        rearNode->next = newNode;
        rearNode = newNode;
    }

    count++;                                      // Increment stored count
    QueueDebugLogger<T>::enqueued(newNode->data); // Log enqueue event
}

// Remove the front element
template <typename T>
void Queue<T>::dequeue()
{
    if (isEmpty())
        throw std::runtime_error("Queue is empty - cannot dequeue");

    Node *temp = frontNode;      // Save pointer to current front
    frontNode = frontNode->next; // Move front pointer forward

    if (frontNode == nullptr)
        rearNode = nullptr; // Queue became empty

    QueueDebugLogger<T>::dequeued(temp->data); // Log before deletion

    delete temp; // Free memory
    count--;     // Reduce size counter
}

// Access front element
template <typename T>
T &Queue<T>::front()
{
    if (isEmpty())
        throw std::runtime_error("Queue is empty - cannot peek front");

    return frontNode->data; // Return existing front element
}

// Check if queue is empty
template <typename T>
bool Queue<T>::isEmpty() const
{
    return frontNode == nullptr;
}

// Return total number of elements
template <typename T>
int Queue<T>::size() const
{
    return count;
}

// Apply a function to each stored element
template <typename T>
template <typename Func>
void Queue<T>::forEach(Func func)
{
    Node *current = frontNode;

    // Traverse list from front to back
    while (current != nullptr)
    {
        func(current->data);     // Invoke callback on value
        current = current->next; // Move to next node
    }
}