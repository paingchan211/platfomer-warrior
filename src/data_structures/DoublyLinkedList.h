#pragma once

#include <functional>
#include <utility>
#include <iostream>
#include "Constants.h"
#include "DoublyLinkedNodeIterator.h"

// Node class for doubly linked list implementation
template <typename DataType>
class DoublyLinkedNode
{
public:
    typedef DoublyLinkedNode<DataType> Node; // Type alias for node type
    static Node NIL;                         // Sentinel node representing end of list or empty list

private:
    DataType value; // Value stored in the node
    Node *next;     // Pointer to the next node in the list
    Node *previous; // Pointer to the previous node in the list

public:
    // Default constructor that initializes node with default value and NIL pointers
    DoublyLinkedNode();

    // Constructor that initializes node with a given value
    explicit DoublyLinkedNode(const DataType &aValue);

    // Move constructor that moves a value into the node
    DoublyLinkedNode(DataType &&aValue);

    // Inserts a new node before this node in the list
    void prepend(Node *newNode);

    // Inserts a new node after this node in the list
    void append(Node *newNode);

    // Returns a copy of the value stored in the node
    DataType getValue() const;

    // Returns a reference to the value stored in the node (non-const)
    DataType &getValueRef();

    // Returns a const reference to the value stored in the node
    const DataType &getValueRef() const;

    // Returns a pointer to the next node
    Node *getNext() const;

    // Returns a pointer to the previous node
    Node *getPrevious() const;

    // Sets the next node pointer
    void setNext(Node *n);

    // Sets the previous node pointer
    void setPrevious(Node *p);

    // Checks if this node is the NIL sentinel node
    bool isNIL() const;
};

// ---------- DoublyLinkedNode Implementation ----------

// Default constructor that initializes node with default value and NIL pointers
template <typename DataType>
DoublyLinkedNode<DataType>::DoublyLinkedNode()
{
    value = DataType();
    next = &NIL;
    previous = &NIL;
}

// Constructor that initializes node with a given value
template <typename DataType>
DoublyLinkedNode<DataType>::DoublyLinkedNode(const DataType &aValue)
{
    value = aValue;
    next = &NIL;
    previous = &NIL;
}

// Move constructor that moves a value into the node
template <typename DataType>
DoublyLinkedNode<DataType>::DoublyLinkedNode(DataType &&aValue)
{
    value = std::move(aValue);
    next = &NIL;
    previous = &NIL;
}

// Inserts a new node before this node in the list
template <typename DataType>
void DoublyLinkedNode<DataType>::prepend(Node *newNode)
{
    // Set new node's next pointer to this node
    newNode->next = this;

    // If this node has a previous node, update its links
    if (this->previous != &NIL)
    {
        newNode->previous = this->previous;
        this->previous->next = newNode;
    }

    // Update this node's previous pointer to point to new node
    this->previous = newNode;
}

// Inserts a new node after this node in the list
template <typename DataType>
void DoublyLinkedNode<DataType>::append(Node *newNode)
{
    // Set new node's previous pointer to this node
    newNode->previous = this;
    // If this node has a next node, update its links
    if (this->next != &NIL)
    {
        newNode->next = this->next;
        this->next->previous = newNode;
    }
    // Update this node's next pointer to point to new node
    this->next = newNode;
}

// Returns a copy of the value stored in the node
template <typename DataType>
DataType DoublyLinkedNode<DataType>::getValue() const
{
    return value;
}

// Returns a reference to the value stored in the node (non-const)
template <typename DataType>
DataType &DoublyLinkedNode<DataType>::getValueRef()
{
    return value;
}

// Returns a const reference to the value stored in the node
template <typename DataType>
const DataType &DoublyLinkedNode<DataType>::getValueRef() const
{
    return value;
}

// Returns a pointer to the next node
template <typename DataType>
typename DoublyLinkedNode<DataType>::Node *DoublyLinkedNode<DataType>::getNext() const
{
    return next;
}

// Returns a pointer to the previous node
template <typename DataType>
typename DoublyLinkedNode<DataType>::Node *DoublyLinkedNode<DataType>::getPrevious() const
{
    return previous;
}

// Sets the next node pointer
template <typename DataType>
void DoublyLinkedNode<DataType>::setNext(Node *n)
{
    next = n;
}

// Sets the previous node pointer
template <typename DataType>
void DoublyLinkedNode<DataType>::setPrevious(Node *p)
{
    previous = p;
}

// Checks if this node is the NIL sentinel node
template <typename DataType>
bool DoublyLinkedNode<DataType>::isNIL() const
{
    return this == &NIL;
}

// Static definition of the NIL sentinel node
template <typename DataType>
DoublyLinkedNode<DataType> DoublyLinkedNode<DataType>::NIL;

// Doubly linked list container class
template <typename T>
class DoublyLinkedList
{
public:
    using Node = DoublyLinkedNode<T>; // Type alias for node type

private:
    Node *head{&Node::NIL}; // Pointer to the first node in the list
    Node *tail{&Node::NIL}; // Pointer to the last node in the list
    int count{0};           // Number of elements in the list

public:
    using iterator = DoublyLinkedNodeIterator<T>; // Iterator type

    // Default constructor that creates an empty list
    DoublyLinkedList() noexcept = default;

    // Move constructor that transfers ownership from another list
    DoublyLinkedList(DoublyLinkedList &&other) noexcept;
    // Move assignment operator that transfers ownership from another list
    DoublyLinkedList &operator=(DoublyLinkedList &&other) noexcept;

    // Copy constructor is deleted (not allowed)
    DoublyLinkedList(const DoublyLinkedList &) = delete;
    // Copy assignment operator is deleted (not allowed)
    DoublyLinkedList &operator=(const DoublyLinkedList &) = delete;

    // Destructor that clears the list
    ~DoublyLinkedList();

    // Checks if the list is empty
    bool isEmpty() const noexcept;
    // Returns the number of elements in the list
    int size() const noexcept;

    // Returns an iterator to the first element (non-const)
    iterator begin() noexcept;
    // Returns an iterator to the end of the list (non-const)
    iterator end() noexcept;

    // Returns a reference to the first element (non-const)
    T &front();
    // Returns a reference to the last element (non-const)
    T &back();

    // Adds an element to the front of the list (move)
    void pushFront(T &&value);
    // Adds an element to the back of the list (move)
    void pushBack(T &&value);
    // Removes the first element from the list
    void popFront();
    // Removes the last element from the list
    void popBack();

    // Removes the element at the specified position
    iterator erase(iterator pos);

    // Removes all elements that satisfy the predicate
    template <typename Predicate>
    int removeIf(Predicate pred);

    // Applies a function to each element in the list (non-const)
    template <typename Func>
    void forEach(Func func);

    // Applies a function to each element in the list (const)
    template <typename Func>
    void forEach(Func func) const;

    // Removes all elements from the list
    void clear() noexcept;
};

// Move constructor that transfers ownership from another list
template <typename T>
DoublyLinkedList<T>::DoublyLinkedList(DoublyLinkedList &&other) noexcept
{
    // Transfer ownership of nodes and size
    head = other.head;
    tail = other.tail;
    count = other.count;
    // Reset other list to empty state
    other.head = other.tail = &Node::NIL;
    other.count = 0;
}

// Move assignment operator that transfers ownership from another list
template <typename T>
DoublyLinkedList<T> &DoublyLinkedList<T>::operator=(DoublyLinkedList &&other) noexcept
{
    // Check for self-assignment
    if (this != &other)
    {
        // Clear this list first
        clear();
        // Transfer ownership of nodes and size
        head = other.head;
        tail = other.tail;
        count = other.count;
        // Reset other list to empty state
        other.head = other.tail = &Node::NIL;
        other.count = 0;
    }
    return *this;
}

// Destructor that clears the list
template <typename T>
DoublyLinkedList<T>::~DoublyLinkedList()
{
    clear();
}

// Checks if the list is empty
template <typename T>
bool DoublyLinkedList<T>::isEmpty() const noexcept
{
    return count == 0;
}

// Returns the number of elements in the list
template <typename T>
int DoublyLinkedList<T>::size() const noexcept
{
    return count;
}

// Returns an iterator to the first element (non-const)
template <typename T>
typename DoublyLinkedList<T>::iterator DoublyLinkedList<T>::begin() noexcept
{
    return iterator(head);
}

// Returns an iterator to the end of the list (non-const)
template <typename T>
typename DoublyLinkedList<T>::iterator DoublyLinkedList<T>::end() noexcept
{
    return iterator(&Node::NIL);
}

// Returns a reference to the first element (non-const)
template <typename T>
T &DoublyLinkedList<T>::front()
{
    // Throw exception if list is empty
    if (head->isNIL())
        throw std::runtime_error("DoublyLinkedList::front on empty list");
    return head->getValueRef();
}

// Returns a reference to the last element (non-const)
template <typename T>
T &DoublyLinkedList<T>::back()
{
    // Throw exception if list is empty
    if (tail->isNIL())
        throw std::runtime_error("DoublyLinkedList::back on empty list");
    return tail->getValueRef();
}

// Adds an element to the front of the list (move)
template <typename T>
void DoublyLinkedList<T>::pushFront(T &&value)
{
    Node *n = new Node(std::move(value));
    // Set new node's pointers
    n->setNext(head);
    n->setPrevious(&Node::NIL);

    // Update head's previous pointer if list is not empty, otherwise set tail
    if (!head->isNIL())
        head->setPrevious(n);
    else
        tail = n;

    // Update head and increment size
    head = n;
    ++count;
}

// Adds an element to the back of the list (move)
template <typename T>
void DoublyLinkedList<T>::pushBack(T &&value)
{
    Node *n = new Node(std::move(value));

    // Set new node's pointers
    n->setPrevious(tail);
    n->setNext(&Node::NIL);

    // Update tail's next pointer if list is not empty, otherwise set head
    if (!tail->isNIL())
        tail->setNext(n);
    else
        head = n;

    // Update tail and increment size
    tail = n;
    ++count;
}

// Removes the first element from the list
template <typename T>
void DoublyLinkedList<T>::popFront()
{
    // Return early if list is empty
    if (head->isNIL())
        return;

    // Save head node and move head to next node
    Node *n = head;
    head = head->getNext();

    // Update links: if new head exists, set its previous to NIL, otherwise set tail to NIL
    if (!head->isNIL())
        head->setPrevious(&Node::NIL);
    else
        tail = &Node::NIL;

    // Delete the old head node and decrement size
    delete n;
    --count;
}

// Removes the last element from the list
template <typename T>
void DoublyLinkedList<T>::popBack()
{
    // Return early if list is empty
    if (tail->isNIL())
        return;

    // Save tail node and move tail to previous node
    Node *n = tail;
    tail = tail->getPrevious();

    // Update links: if new tail exists, set its next to NIL, otherwise set head to NIL
    if (!tail->isNIL())
        tail->setNext(&Node::NIL);
    else
        head = &Node::NIL;

    // Delete the old tail node and decrement size
    delete n;
    --count;
}

// Removes the element at the specified position
template <typename T>
typename DoublyLinkedList<T>::iterator
DoublyLinkedList<T>::erase(iterator pos)
{
    Node *current = pos.node;
    // Return end iterator if position is NIL
    if (current->isNIL())
        return end();

    // Get previous and next nodes
    Node *prev = current->getPrevious();
    Node *next = current->getNext();

    // Update previous node's next pointer, or update head if at beginning
    if (!prev->isNIL())
        prev->setNext(next);
    else
        head = next;

    // Update next node's previous pointer, or update tail if at end
    if (!next->isNIL())
        next->setPrevious(prev);
    else
        tail = prev;

    // Save iterator to next node before deleting
    iterator ret(next);
    delete current;
    --count;

    return ret;
}

// Removes all elements that satisfy the predicate
template <typename T>
template <typename Predicate>
int DoublyLinkedList<T>::removeIf(Predicate pred)
{
    int removed = 0;
    Node *cur = head;

    // Iterate through all nodes
    while (!cur->isNIL())
    {
        // Save next node before potentially deleting current
        Node *nxt = cur->getNext();

        // Check if current node satisfies the predicate
        if (pred(cur->getValueRef()))
        {
            Node *prev = cur->getPrevious();

            // Update previous node's next pointer, or update head if at beginning
            if (!prev->isNIL())
                prev->setNext(cur->getNext());
            else
                head = cur->getNext();

            // Update next node's previous pointer, or update tail if at end
            if (!cur->getNext()->isNIL())
                cur->getNext()->setPrevious(prev);
            else
                tail = prev;

            // Delete node and update counters
            delete cur;
            --count;
            ++removed;
        }

        // Move to next node
        cur = nxt;
    }

    return removed;
}

// Applies a function to each element in the list (non-const)
template <typename T>
template <typename Func>
void DoublyLinkedList<T>::forEach(Func func)
{
    Node *cur = head;
    int elementCount = 0;

    // Iterate through all nodes and apply function
    while (!cur->isNIL())
    {
        func(cur->getValueRef());
        cur = cur->getNext();
        elementCount++;
    }

    if (ENABLE_ITERATOR_STDOUT && elementCount > 0)
        std::cout << "[Iterator] forEach() processed " << elementCount << " elements" << std::endl;
}

// Applies a function to each element in the list (const)
template <typename T>
template <typename Func>
void DoublyLinkedList<T>::forEach(Func func) const
{
    Node *cur = head;

    // Iterate through all nodes and apply function
    while (!cur->isNIL())
    {
        func(cur->getValueRef());
        cur = cur->getNext();
    }
}

// Removes all elements from the list
template <typename T>
void DoublyLinkedList<T>::clear() noexcept
{
    Node *cur = head;

    // Delete all nodes in the list
    while (!cur->isNIL())
    {
        Node *nxt = cur->getNext();
        delete cur;
        cur = nxt;
    }

    // Reset list to empty state
    head = tail = &Node::NIL;
    count = 0;
}