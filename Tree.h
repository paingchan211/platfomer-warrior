#pragma once

#include <stdexcept>
#include <functional>

// Template class for an N-ary tree (each node can have up to N children)
template <class T, int N>
class NTree
{
private:
    T key;                 // Key value of this node
    NTree<T, N> *nodes[N]; // Array of child pointers
    NTree<T, N> *parent_;  // Pointer to parent node
    NTree();               // Private default constructor

public:
    static NTree<T, N> NIL; // Sentinel NIL node (represents empty)

    explicit NTree(const T &key); // Constructor with key
    ~NTree();                     // Destructor

    // Disable copying and moving
    NTree(const NTree &) = delete;
    NTree &operator=(const NTree &) = delete;
    NTree(NTree &&) = delete;
    NTree &operator=(NTree &&) = delete;

    // Basic accessors
    bool isEmpty() const;
    const T &getKey() const;
    T &getKey();

    // Index operator to access children
    NTree<T, N> &operator[](int index) const;

    // Attach and detach child trees
    void attachNTree(int index, NTree<T, N> *tree);
    NTree<T, N> *detachNTree(int index);

    // Utility functions
    int getSize() const;
    NTree<T, N> *search(const T &key) const;

    // Parent and root accessors
    NTree<T, N> *getParent() const { return parent_; }
    bool isRoot() const { return parent_ == nullptr || parent_ == &NIL; }

    // Traversal functions
    void traverseLevelOrder(std::function<void(T &)> visitor);
    void traverseLevelOrder(std::function<void(const T &)> visitor) const;
};

// Define static NIL sentinel
template <class T, int N>
NTree<T, N> NTree<T, N>::NIL;

// Default constructor initializes children as NIL
template <class T, int N>
NTree<T, N>::NTree() : parent_(nullptr)
{
    for (int i = 0; i < N; ++i)
        nodes[i] = &NIL;
}

// Constructor with key
template <class T, int N>
NTree<T, N>::NTree(const T &aKey) : key(aKey), parent_(nullptr)
{
    for (int i = 0; i < N; ++i)
        nodes[i] = &NIL;
}

// Destructor deletes all children
template <class T, int N>
NTree<T, N>::~NTree()
{
    for (int i = 0; i < N; ++i)
    {
        if (nodes[i] != &NIL)
            delete nodes[i];
    }
}

// Check if current node is NIL
template <class T, int N>
bool NTree<T, N>::isEmpty() const
{
    return this == &NIL;
}

// Return key (const)
template <class T, int N>
const T &NTree<T, N>::getKey() const
{
    if (isEmpty())
        throw std::domain_error("Empty NTree!");
    return key;
}

// Return key (mutable)
template <class T, int N>
T &NTree<T, N>::getKey()
{
    if (isEmpty())
        throw std::domain_error("Empty NTree!");
    return key;
}

// Access child by index
template <class T, int N>
NTree<T, N> &NTree<T, N>::operator[](int index) const
{
    if (isEmpty())
        throw std::domain_error("Empty NTree!");
    if (index < 0 || index >= N)
        throw std::out_of_range("Illegal subtree index");
    return *nodes[index];
}

// Attach a child node at index
template <class T, int N>
void NTree<T, N>::attachNTree(int index, NTree<T, N> *tree)
{
    if (isEmpty())
        throw std::domain_error("Empty NTree!");
    if (index < 0 || index >= N)
        throw std::out_of_range("Illegal subtree index");
    if (nodes[index] != &NIL)
        throw std::domain_error("Non-empty subtree present!");

    nodes[index] = tree;
    if (tree != &NIL)
        tree->parent_ = this;
}

// Detach and return child tree
template <class T, int N>
NTree<T, N> *NTree<T, N>::detachNTree(int index)
{
    if (isEmpty())
        throw std::domain_error("Empty NTree!");
    if (index < 0 || index >= N)
        throw std::out_of_range("Illegal subtree index");

    NTree<T, N> *detached = nodes[index];
    nodes[index] = &NIL;
    if (detached != &NIL)
        detached->parent_ = nullptr;
    return detached;
}

// Recursively count size of entire tree
template <class T, int N>
int NTree<T, N>::getSize() const
{
    if (isEmpty())
        return 0;

    int size = 1;
    for (int i = 0; i < N; ++i)
        size += nodes[i]->getSize();

    return size;
}

// Search tree recursively for a key
template <class T, int N>
NTree<T, N> *NTree<T, N>::search(const T &aKey) const
{
    if (isEmpty())
        return nullptr;

    if (key == aKey)
        return const_cast<NTree<T, N> *>(this);

    for (int i = 0; i < N; ++i)
    {
        NTree<T, N> *result = nodes[i]->search(aKey);
        if (result)
            return result;
    }
    return nullptr;
}

// Traverse tree in level order (non-const)
template <class T, int N>
void NTree<T, N>::traverseLevelOrder(std::function<void(T &)> visitor)
{
    if (isEmpty())
        return;

    // Simple queue implementation for traversal
    struct SimpleQueue
    {
        struct QNode
        {
            NTree<T, N> *node;
            QNode *next;
            QNode(NTree<T, N> *n) : node(n), next(nullptr) {}
        };

        QNode *head = nullptr;
        QNode *tail = nullptr;

        void push(NTree<T, N> *n)
        {
            if (n == &NTree<T, N>::NIL)
                return;
            QNode *q = new QNode(n);
            if (!tail)
                head = tail = q;
            else
            {
                tail->next = q;
                tail = q;
            }
        }

        NTree<T, N> *pop()
        {
            if (!head)
                return nullptr;
            QNode *q = head;
            NTree<T, N> *n = q->node;
            head = head->next;
            if (!head)
                tail = nullptr;
            delete q;
            return n;
        }

        bool empty() const { return head == nullptr; }

        ~SimpleQueue()
        {
            while (head)
            {
                QNode *tmp = head;
                head = head->next;
                delete tmp;
            }
        }
    };

    SimpleQueue queue;
    queue.push(this);

    while (!queue.empty())
    {
        NTree<T, N> *current = queue.pop();
        if (!current || current->isEmpty())
            continue;

        visitor(current->key);

        for (int i = 0; i < N; ++i)
        {
            if (current->nodes[i] != &NIL)
                queue.push(current->nodes[i]);
        }
    }
}

// Traverse tree in level order (const version)
template <class T, int N>
void NTree<T, N>::traverseLevelOrder(std::function<void(const T &)> visitor) const
{
    if (isEmpty())
        return;

    // Const queue implementation for traversal
    struct SimpleQueue
    {
        struct QNode
        {
            const NTree<T, N> *node;
            QNode *next;
            QNode(const NTree<T, N> *n) : node(n), next(nullptr) {}
        };

        QNode *head = nullptr;
        QNode *tail = nullptr;

        void push(const NTree<T, N> *n)
        {
            if (n == &NTree<T, N>::NIL)
                return;
            QNode *q = new QNode(n);
            if (!tail)
                head = tail = q;
            else
            {
                tail->next = q;
                tail = q;
            }
        }

        const NTree<T, N> *pop()
        {
            if (!head)
                return nullptr;
            QNode *q = head;
            const NTree<T, N> *n = q->node;
            head = head->next;
            if (!head)
                tail = nullptr;
            delete q;
            return n;
        }

        bool empty() const { return head == nullptr; }

        ~SimpleQueue()
        {
            while (head)
            {
                QNode *tmp = head;
                head = head->next;
                delete tmp;
            }
        }
    };

    SimpleQueue queue;
    queue.push(this);

    while (!queue.empty())
    {
        const NTree<T, N> *current = queue.pop();
        if (!current || current->isEmpty())
            continue;

        visitor(current->key);

        for (int i = 0; i < N; ++i)
        {
            if (current->nodes[i] != &NIL)
                queue.push(current->nodes[i]);
        }
    }
}