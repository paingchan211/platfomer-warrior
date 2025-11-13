#pragma once

#include <stdexcept>
#include <utility>

// Simple templated linked-list queue implementation
template <typename T>
class Queue
{
private:
    // Node struct represents one queue element
    struct Node
    {
        T data;     // Stored data
        Node *next; // Pointer to next node

        Node(const T &value) : data(value), next(nullptr) {}
        Node(T &&value) : data(std::move(value)), next(nullptr) {}
    };

    Node *frontNode; // Front of queue
    Node *rearNode;  // Rear of queue
    int count;       // Number of elements

public:
    // Default constructor initializes empty queue
    Queue() : frontNode(nullptr), rearNode(nullptr), count(0) {}

    // Destructor clears all elements
    ~Queue()
    {
        clear();
    }

    // Add element by copy
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
    }

    // Add element by move
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
    }

    // Remove front element
    void dequeue()
    {
        if (isEmpty())
        {
            throw std::runtime_error("Queue is empty - cannot dequeue");
        }

        Node *temp = frontNode;
        frontNode = frontNode->next;

        if (frontNode == nullptr)
        {
            rearNode = nullptr;
        }

        delete temp;
        count--;
    }

    // Return reference to front element
    T &front()
    {
        if (isEmpty())
        {
            throw std::runtime_error("Queue is empty - cannot peek front");
        }
        return frontNode->data;
    }

    // Const version of front()
    const T &front() const
    {
        if (isEmpty())
        {
            throw std::runtime_error("Queue is empty - cannot peek front");
        }
        return frontNode->data;
    }

    // Check if queue is empty
    bool isEmpty() const
    {
        return frontNode == nullptr;
    }

    // Get queue size
    int size() const
    {
        return count;
    }

    // Remove all elements
    void clear()
    {
        while (!isEmpty())
        {
            dequeue();
        }
    }

    // Apply function to each element
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

    // Remove elements satisfying predicate
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
                if (previous == nullptr)
                {
                    frontNode = next;
                    if (frontNode == nullptr)
                    {
                        rearNode = nullptr;
                    }
                }
                else
                {
                    previous->next = next;
                    if (current == rearNode)
                    {
                        rearNode = previous;
                    }
                }

                delete current;
                count--;
            }
            else
            {
                previous = current;
            }

            current = next;
        }
    }
};