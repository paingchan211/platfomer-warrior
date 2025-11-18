#pragma once

#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>
#include "FloatingText.h"

// A small debugging helper. By default it does nothing, but specific types
// can specialize it to print logs when items are added or removed.
template <typename T>
struct QueueDebugLogger
{
    static void enqueued(const T &) {}
    static void dequeued(const T &) {}
};

// Specialization for std::unique_ptr<FloatingText> so enqueue/dequeue events
// print the text that was added or removed.
template <>
struct QueueDebugLogger<std::unique_ptr<FloatingText>>
{
    static void enqueued(const std::unique_ptr<FloatingText> &value)
    {
        if (value)
            std::cout << "[Queue] '" << value->getDisplayString()
                      << "' enqueued\n";
    }

    static void dequeued(const std::unique_ptr<FloatingText> &value)
    {
        if (value)
            std::cout << "[Queue] '" << value->getDisplayString()
                      << "' dequeued\n";
    }
};

// A simple FIFO queue implemented using a singly linked list.
// Supports enqueue, dequeue, front(), iterating through elements,
// and removing elements that match a predicate.
template <typename T>
class Queue
{
private:
    // A single node in the linked list. Stores its value and a pointer
    // to the next node in the queue.
    struct Node
    {
        T data;
        Node *next;

        // Store a copy of the value.
        Node(const T &value) : data(value), next(nullptr) {}

        // Move the value into the node (avoids copying).
        Node(T &&value) : data(std::move(value)), next(nullptr) {}
    };

    Node *frontNode; // First element in queue (oldest element)
    Node *rearNode;  // Last element in queue (most recently inserted)
    int count;       // Total number of stored elements

public:
    // Constructs an empty queue with no nodes.
    Queue() : frontNode(nullptr), rearNode(nullptr), count(0) {}

    // Destructor clears all nodes to avoid leaking memory.
    ~Queue()
    {
        clear();
    }

    // Enqueue by copying. Creates a new node and appends it to the rear.
    void enqueue(const T &value)
    {
        Node *newNode = new Node(value);

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

    // Enqueue by moving. Useful for large or non-copyable types.
    void enqueue(T &&value)
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

    // Removes the front element. Throws if queue is empty.
    void dequeue()
    {
        if (isEmpty())
            throw std::runtime_error("Queue is empty - cannot dequeue");

        Node *temp = frontNode;
        frontNode = frontNode->next;

        // If queue became empty after removing, rear also must become null.
        if (frontNode == nullptr)
            rearNode = nullptr;

        QueueDebugLogger<T>::dequeued(temp->data);
        delete temp;
        count--;
    }

    // Returns a reference to the value at the front (modifiable).
    T &front()
    {
        if (isEmpty())
            throw std::runtime_error("Queue is empty - cannot peek front");

        return frontNode->data;
    }

    // Returns a const reference to the value at the front.
    const T &front() const
    {
        if (isEmpty())
            throw std::runtime_error("Queue is empty - cannot peek front");

        return frontNode->data;
    }

    // True if there are no nodes.
    bool isEmpty() const
    {
        return frontNode == nullptr;
    }

    // Number of elements stored.
    int size() const
    {
        return count;
    }

    // Removes every element from the queue by repeatedly calling dequeue.
    void clear()
    {
        while (!isEmpty())
            dequeue();
    }

    // Applies a function to every stored element, in order from front to back.
    template <typename Func>
    void forEach(Func func)
    {
        Node *current = frontNode;
        while (current != nullptr)
        {
            func(current->data);
            current = current->next;
        }
    }

    // Removes all nodes for which pred(value) returns true.
    // Handles removing from:
    // - the front
    // - the middle
    // - the rear
    template <typename Predicate>
    void removeIf(Predicate pred)
    {
        Node *current = frontNode;
        Node *previous = nullptr;

        while (current != nullptr)
        {
            Node *next = current->next;

            if (pred(current->data))
            {
                // Removing the first node
                if (previous == nullptr)
                {
                    frontNode = next;

                    // If the list becomes empty, update rearNode
                    if (frontNode == nullptr)
                        rearNode = nullptr;
                }
                else
                {
                    // Removing node in the middle or rear
                    previous->next = next;

                    // If removing the last node, update rear pointer
                    if (current == rearNode)
                        rearNode = previous;
                }

                delete current;
                count--;
            }
            else
            {
                // If node is kept, advance previous pointer
                previous = current;
            }

            current = next;
        }
    }
};