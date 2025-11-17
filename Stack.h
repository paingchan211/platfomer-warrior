#pragma once

#include <stdexcept> // For throwing exceptions
#include "Constants.h"

// Forward declarations so Stack can emit debug info without requiring the full header here
enum class GameStateType;
struct GameStateData;
const char *toString(GameStateType type);

// Default logger does nothing for most stack element types
template <typename T>
struct StackDebugLogger
{
    static void pushed(const T &) {}
    static void popped(const T &) {}
};

// Specialization for GameStateData so we can trace the session state stack
template <>
struct StackDebugLogger<GameStateData>
{
    static void pushed(const GameStateData &state);
    static void popped(const GameStateData &state);
};

// Template class implementing a basic stack using a singly linked list.
// Stores elements of type T. Supports move semantics but disables copying.
template <typename T>
class Stack
{
private:
    // Node represents one element in the stack. Each node contains:
    // - data: the stored value (T)
    // - next: pointer to the node below it
    struct Node
    {
        T data;
        Node *next;

        // Construct node by *copying* T
        Node(const T &value, Node *nextNode = nullptr);

        // Construct node by *moving* T
        Node(T &&value, Node *nextNode = nullptr);
    };

    Node *topNode; // Pointer to top of stack (nullptr when empty)
    int count;     // Total number of nodes in the stack

public:
    Stack();  // Default constructor
    ~Stack(); // Destructor

    // Disable copy operations to prevent accidental deep copies (expensive)
    Stack(const Stack &) = delete;
    Stack &operator=(const Stack &) = delete;

    // Enable move operations for efficiency
    Stack(Stack &&other) noexcept;
    Stack &operator=(Stack &&other) noexcept;

    // Pushing values onto stack
    void push(const T &value); // Copy push
    void push(T &&value);      // Move push

    // Pop and top operations
    void pop(); // Remove top element; throws if empty

    T &top();             // Access top element (mutable)
    const T &top() const; // Access top element (const)

    // Peek allows reading an element deeper in the stack (0 = top)
    T &peek(int level);
    const T &peek(int level) const;

    // Stack status
    bool isEmpty() const;
    int size() const;
    void clear(); // Remove all nodes from stack
};

// Node copy constructor
template <typename T>
Stack<T>::Node::Node(const T &value, Node *nextNode)
    : data(value),   // Copy the provided value into node
      next(nextNode) // Link to next node (can be nullptr)
{
}

// Node move constructor
template <typename T>
Stack<T>::Node::Node(T &&value, Node *nextNode)
    : data(std::move(value)), // Move the value (avoids copying heavy objects)
      next(nextNode)
{
}

// Default constructor initializes stack as empty
template <typename T>
Stack<T>::Stack()
    : topNode(nullptr), // No nodes initially
      count(0)
{
}

// Destructor ensures all allocated nodes are deallocated
template <typename T>
Stack<T>::~Stack()
{
    clear(); // Clean up all nodes
}

// Move constructor: transfer ownership of node chain
template <typename T>
Stack<T>::Stack(Stack &&other) noexcept
    : topNode(other.topNode), // Steal other's nodes
      count(other.count)
{
    other.topNode = nullptr;
    other.count = 0;
}

// Move assignment: clear current stack then take ownership from other
template <typename T>
Stack<T> &Stack<T>::operator=(Stack &&other) noexcept
{
    if (this != &other) // Prevent self-move
    {
        clear(); // Free current nodes

        topNode = other.topNode;
        count = other.count;

        other.topNode = nullptr;
        other.count = 0;
    }
    return *this;
}

// Push an element onto the stack by copying the value
template <typename T>
void Stack<T>::push(const T &value)
{
    Node *newNode = new Node(value, topNode); // Allocate new node
    topNode = newNode;                        // New node becomes the top
    ++count;
    StackDebugLogger<T>::pushed(newNode->data);
}

// Push an element onto the stack by moving the value
template <typename T>
void Stack<T>::push(T &&value)
{
    Node *newNode = new Node(std::move(value), topNode);
    topNode = newNode;
    ++count;
    StackDebugLogger<T>::pushed(newNode->data);
}

// Pop removes the top element from the stack
template <typename T>
void Stack<T>::pop()
{
    if (isEmpty())
        throw std::runtime_error("Stack::pop() called on empty stack");

    Node *oldTop = topNode;  // Save pointer to delete
    topNode = topNode->next; // Move top downward
    StackDebugLogger<T>::popped(oldTop->data);
    delete oldTop;           // Free removed node
    --count;
}

// Get reference to top element (mutable)
template <typename T>
T &Stack<T>::top()
{
    if (isEmpty())
        throw std::runtime_error("Stack::top() called on empty stack");

    return topNode->data;
}

// Get reference to top element (const)
template <typename T>
const T &Stack<T>::top() const
{
    if (isEmpty())
        throw std::runtime_error("Stack::top() called on empty stack");

    return topNode->data;
}

// Peek returns the element 'level' below the top (0 = top)
template <typename T>
T &Stack<T>::peek(int level)
{
    if (level < 0 || level >= count)
        throw std::out_of_range("Stack::peek() level out of range");

    Node *current = topNode;

    // Traverse downward to "level"
    for (int i = 0; i < level; ++i)
        current = current->next;

    return current->data;
}

// Const version of peek
template <typename T>
const T &Stack<T>::peek(int level) const
{
    if (level < 0 || level >= count)
        throw std::out_of_range("Stack::peek() level out of range");

    Node *current = topNode;
    for (int i = 0; i < level; ++i)
        current = current->next;

    return current->data;
}

// Returns true if stack has no nodes
template <typename T>
bool Stack<T>::isEmpty() const
{
    return topNode == nullptr;
}

// Returns number of stored elements
template <typename T>
int Stack<T>::size() const
{
    return count;
}

// Deletes all nodes and resets stack to empty state
template <typename T>
void Stack<T>::clear()
{
    while (!isEmpty())
    {
        Node *temp = topNode;
        topNode = topNode->next;
        delete temp; // Delete each node
    }

    count = 0;
}
