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
    struct Node
    {
        T data;     // Stored payload (e.g., combat log entry)
        Node *next; // Link to the next element in the chain

        // Construct a node and optionally chain it to the next node.
        Node(T value, Node *nextNode = nullptr);
    };

    Node *head; // Oldest element (front of the queue)
    Node *tail; // Most recently appended element
    int count;  // Number of nodes currently stored

    // Helper used by copy ctor/assignment to clone another list element by element.
    void copyFrom(const SinglyLinkedList &other);
    // Exchange head/tail/count pointers in O(1); core of copy-and-swap.
    void swap(SinglyLinkedList &other) noexcept;

public:
    // Construct an empty list.
    SinglyLinkedList();
    // Deep copy another list.
    SinglyLinkedList(const SinglyLinkedList &other);
    // Copy assignment (copy-and-swap idiom).
    SinglyLinkedList &operator=(const SinglyLinkedList &other);
    // Move construct from another list, leaving it empty.
    SinglyLinkedList(SinglyLinkedList &&other) noexcept;
    // Move assignment steals another list's nodes.
    SinglyLinkedList &operator=(SinglyLinkedList &&other) noexcept;
    // Destroy all nodes on destruction.
    ~SinglyLinkedList();

    // Append element at the tail in O(1).
    void pushBack(T value);
    // Prepend element at the head in O(1).
    void pushFront(T value);
    // Remove the head node; throws if empty.
    void popFront();
    // Remove element at index (O(n)); returns success flag.
    bool removeAt(int index);
    // Remove all entries that satisfy predicate; returns count removed.
    template <typename Predicate>
    int removeIf(Predicate predicate);
    // Apply functor to each entry (mutable access).
    template <typename Func>
    void forEach(Func func);
    // Apply functor to each entry (read-only).
    template <typename Func>
    void forEach(Func func) const;
    // Destroy every node and reset to empty.
    void clear();
    // True when the list has no nodes.
    bool isEmpty() const;
    // Number of stored elements.
    int size() const;
    // Oldest element (front).
    T front() const;
    // Newest element (back).
    T back() const;
};

// ---------- Implementation ----------

// Build a node by copying/moving value and storing optional next pointer.
template <typename T>
SinglyLinkedList<T>::Node::Node(T value, Node *nextNode) : data(value), next(nextNode) {}

// Start empty with null head/tail and zero count.
template <typename T>
SinglyLinkedList<T>::SinglyLinkedList() : head(nullptr), tail(nullptr), count(0) {}

template <typename T>
SinglyLinkedList<T>::SinglyLinkedList(const SinglyLinkedList &other) : head(nullptr), tail(nullptr), count(0)
{
    copyFrom(other); // Reuse helper so logic stays consistent with assignment
}

template <typename T>
SinglyLinkedList<T> &SinglyLinkedList<T>::operator=(const SinglyLinkedList &other)
{
    if (this != &other)
    {
        SinglyLinkedList tmp(other); // Copy first so failure leaves *this untouched
        swap(tmp);                   // Swap contents; tmp cleans up old data in destructor
    }
    return *this;
}

template <typename T>
SinglyLinkedList<T>::SinglyLinkedList(SinglyLinkedList &&other) noexcept
    : head(other.head), tail(other.tail), count(other.count)
{
    // Leave the moved-from list empty so destruction is safe.
    other.head = nullptr;
    other.tail = nullptr;
    other.count = 0;
}

template <typename T>
SinglyLinkedList<T> &SinglyLinkedList<T>::operator=(SinglyLinkedList &&other) noexcept
{
    if (this != &other)
    {
        clear(); // Release current nodes before taking ownership
        head = other.head;
        tail = other.tail;
        count = other.count;

        other.head = nullptr;
        other.tail = nullptr;
        other.count = 0;
    }
    return *this;
}

template <typename T>
SinglyLinkedList<T>::~SinglyLinkedList()
{
    clear();
}

template <typename T>
void SinglyLinkedList<T>::pushBack(T value)
{
    Node *newNode = new Node(value); // Allocate node that will become the new tail

    if (tail == nullptr)
    {
        head = tail = newNode; // First insertion populates both head and tail
    }
    else
    {
        tail->next = newNode; // Link previous tail to the new node
        tail = newNode;       // Update cached tail pointer
    }
    ++count;
}

template <typename T>
void SinglyLinkedList<T>::pushFront(T value)
{
    Node *newNode = new Node(value, head); // New node points to current head
    head = newNode;                        // Head now references new node

    if (tail == nullptr)
    {
        tail = newNode; // If list was empty, tail must also point to the node
    }
    ++count;
}

template <typename T>
void SinglyLinkedList<T>::popFront()
{
    if (head == nullptr)
    {
        throw std::runtime_error("Cannot pop from empty list");
    }

    Node *toDelete = head; // Remember current head so we can delete it
    head = head->next;     // Advance head to the next node

    if (head == nullptr)
    {
        tail = nullptr; // Clearing last element resets tail as well
    }

    delete toDelete;
    --count;
}

template <typename T>
bool SinglyLinkedList<T>::removeAt(int index)
{
    if (index < 0 || index >= count)
    {
        return false; // Out of range, nothing to remove
    }

    if (index == 0)
    {
        popFront();
        return true;
    }

    Node *previous = head;
    for (int i = 0; i < index - 1; ++i) // Walk to node prior to the one we remove
    {
        previous = previous->next;
    }

    Node *toDelete = previous->next;
    previous->next = toDelete->next;

    if (toDelete == tail)
    {
        tail = previous;
    }

    delete toDelete;
    --count;
    return true;
}

template <typename T>
template <typename Predicate>
int SinglyLinkedList<T>::removeIf(Predicate predicate)
{
    Node *current = head;     // Node currently under inspection
    Node *previous = nullptr; // Node immediately before current
    int removed = 0;          // Counter to report removals

    while (current != nullptr)
    {
        if (predicate(current->data))
        {
            Node *toDelete = current;

            if (previous == nullptr)
            {
                head = current->next; // Removing head
                current = head;

                if (head == nullptr)
                    tail = nullptr; // List is now empty
            }
            else
            {
                previous->next = current->next;
                current = current->next;

                if (toDelete == tail)
                    tail = previous; // Removed tail; update cached pointer
            }

            delete toDelete;
            --count;
            ++removed;
        }
        else
        {
            previous = current;      // Advance previous pointer
            current = current->next; // Continue iteration
        }
    }
    return removed;
}

template <typename T>
template <typename Func>
void SinglyLinkedList<T>::forEach(Func func)
{
    Node *current = head;

    while (current != nullptr)
    {
        func(current->data);     // Invoke callback with mutable reference
        current = current->next; // Traverse to next element
    }
}

template <typename T>
template <typename Func>
void SinglyLinkedList<T>::forEach(Func func) const
{
    Node *current = head;

    while (current != nullptr)
    {
        func(current->data);     // Invoke callback in read-only context
        current = current->next;
    }
}

template <typename T>
void SinglyLinkedList<T>::clear()
{
    Node *current = head; // Start from head and delete sequentially

    while (current != nullptr)
    {
        Node *next = current->next; // Cache next before deleting current
        delete current;
        current = next;
    }

    head = tail = nullptr; // Reset to empty sentinel state
    count = 0;
}

template <typename T>
bool SinglyLinkedList<T>::isEmpty() const { return head == nullptr; }

template <typename T>
int SinglyLinkedList<T>::size() const { return count; }

template <typename T>
T SinglyLinkedList<T>::front() const
{
    if (head == nullptr)
        throw std::runtime_error("Cannot get front of empty list");
    return head->data; // Return copy of oldest element
}

template <typename T>
T SinglyLinkedList<T>::back() const
{
    if (tail == nullptr)
        throw std::runtime_error("Cannot get back of empty list");
    return tail->data; // Return copy of newest element
}

template <typename T>
void SinglyLinkedList<T>::swap(SinglyLinkedList<T> &other) noexcept
{
    std::swap(this->head, other.head);   // Exchange head pointers
    std::swap(this->tail, other.tail);   // Exchange tail pointers
    std::swap(this->count, other.count); // Exchange node counts
}

template <typename T>
void SinglyLinkedList<T>::copyFrom(const SinglyLinkedList<T> &other)
{
    Node *current = other.head; // Walk the source list from head to tail

    while (current != nullptr)
    {
        pushBack(current->data); // Reuse pushBack to preserve order
        current = current->next;
    }
}
