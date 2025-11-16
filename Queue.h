#pragma once

#include <stdexcept>
#include <utility>

// Simple templated linked-list queue implementation.
// Supports enqueue/dequeue, iteration through forEach, and conditional removal.
template <typename T>
class Queue
{
private:
    // Node struct represents one queue element
    struct Node
    {
        T data;     // Stored data value
        Node *next; // Pointer to next node in the queue

        Node(const T &value); // Copy-construct node from value
        Node(T &&value);      // Move-construct node from value
    };

    Node *frontNode; // Pointer to current front element
    Node *rearNode;  // Pointer to current rear element
    int count;       // Number of stored elements

public:
    Queue();  // Default constructor initializes empty queue.
    ~Queue(); // Destructor clears all elements to avoid leaks.

    void enqueue(const T &value); // Add element by copy to the back
    void enqueue(T &&value);      // Add element by move to the back

    void dequeue(); // Remove front element, throwing if empty

    T &front();             // Non-const access to front element
    const T &front() const; // Const access to front element

    bool isEmpty() const; // Check if queue is empty
    int size() const;     // Get queue size
    void clear();         // Remove all elements

    template <typename Func>
    void forEach(Func func); // Apply function to each element

    template <typename Predicate>
    void removeIf(Predicate pred); // Remove elements satisfying predicate
};

// ---------- Implementation ----------

// Copy-construct node from value and point to null by default.
template <typename T>
Queue<T>::Node::Node(const T &value) : data(value), next(nullptr) {}

// Move-construct node from value.
template <typename T>
Queue<T>::Node::Node(T &&value) : data(std::move(value)), next(nullptr) {}

// Default constructor initializes empty queue.
template <typename T>
Queue<T>::Queue() : frontNode(nullptr), rearNode(nullptr), count(0) {}

// Destructor clears all elements to avoid leaks.
template <typename T>
Queue<T>::~Queue()
{
    clear();
}

// Add element by copy (value preserved) to the back of the queue.
template <typename T>
void Queue<T>::enqueue(const T &value)
{
    Node *newNode = new Node(value);

    if (isEmpty())
    {
        frontNode = rearNode = newNode; // first element sets both pointers
    }
    else
    {
        rearNode->next = newNode;
        rearNode = newNode;
    }

    count++;
}

// Add element by move (value reused) to the back of the queue.
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
}

// Remove front element, throwing if queue is empty.
template <typename T>
void Queue<T>::dequeue()
{
    if (isEmpty())
    {
        throw std::runtime_error("Queue is empty - cannot dequeue");
    }

    Node *temp = frontNode;
    frontNode = frontNode->next;

    if (frontNode == nullptr)
    {
        rearNode = nullptr; // queue became empty, reset rear as well
    }

    delete temp;
    count--;
}

// Return reference to front element (non-const).
template <typename T>
T &Queue<T>::front()
{
    if (isEmpty())
    {
        throw std::runtime_error("Queue is empty - cannot peek front");
    }
    return frontNode->data;
}

// Const version of front(), still throwing if empty.
template <typename T>
const T &Queue<T>::front() const
{
    if (isEmpty())
    {
        throw std::runtime_error("Queue is empty - cannot peek front");
    }
    return frontNode->data;
}

// Check if queue is empty.
template <typename T>
bool Queue<T>::isEmpty() const
{
    return frontNode == nullptr;
}

// Get queue size (number of stored items).
template <typename T>
int Queue<T>::size() const
{
    return count;
}

// Remove all elements by repeatedly dequeuing.
template <typename T>
void Queue<T>::clear()
{
    while (!isEmpty())
    {
        dequeue();
    }
}

// Apply function to each element in order without modifying the queue.
template <typename T>
template <typename Func>
void Queue<T>::forEach(Func func)
{
    Node *current = frontNode;
    while (current != nullptr)
    {
        func(current->data);
        current = current->next;
    }
}

// Remove elements satisfying predicate while preserving relative order of remaining nodes.
template <typename T>
template <typename Predicate>
void Queue<T>::removeIf(Predicate pred)
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
