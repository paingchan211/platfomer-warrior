#pragma once

#include <stdexcept>  // For domain_error and out_of_range exceptions
#include <functional> // For std::function visitor callbacks

// Generic N-ary tree node, where each node can have up to N children.
// The class uses a static NIL node to represent "empty" nodes, reducing
// null-pointer checks and allowing each child pointer to always be valid.
template <class T, int N>
class NTree
{
private:
    T key;                 // The value stored at this node
    NTree<T, N> *nodes[N]; // Fixed-size array of child pointers
    NTree<T, N> *parent_;  // Pointer to this node's parent (nullptr for root)
    NTree();               // Private default constructor used only for defining NIL

public:
    static NTree<T, N> NIL; // Global NIL sentinel acting as an "empty node"

    explicit NTree(const T &key); // Construct a node containing 'key'
    ~NTree();                     // Destructor recursively deletes owned children

    // Copy and move operations disabled to avoid accidentally duplicating pointers
    NTree(const NTree &) = delete;
    NTree &operator=(const NTree &) = delete;
    NTree(NTree &&) = delete;
    NTree &operator=(NTree &&) = delete;

    // Check whether this node *is* the NIL sentinel
    bool isEmpty() const;

    // Access stored key (throws if node is NIL)
    const T &getKey() const;
    T &getKey();

    // Access a child subtree by index (0 to N-1)
    NTree<T, N> &operator[](int index) const;

    // Attach or detach a subtree at a given index
    void attachNTree(int index, NTree<T, N> *tree);
    NTree<T, N> *detachNTree(int index);

    // Recursively count total nodes in this subtree
    int getSize() const;

    // Recursively search subtree for a key, returns pointer or nullptr
    NTree<T, N> *search(const T &key) const;

    // Parent accessors
    NTree<T, N> *getParent() const { return parent_; }

    // True if parent is null or this node *is* the root sentinel
    bool isRoot() const { return parent_ == nullptr || parent_ == &NTree<T, N>::NIL; }

    // Level-order (breadth-first) traversal with read/write or read-only callback
    void traverseLevelOrder(std::function<void(T &)> visitor);
    void traverseLevelOrder(std::function<void(const T &)> visitor) const;
};

// -------------------- Static NIL sentinel definition --------------------

// Define a single global empty node for all NTree<T, N>. This node’s children
// all point back to itself, representing an empty or absent child.
template <class T, int N>
NTree<T, N> NTree<T, N>::NIL;

// -------------------- Private default constructor --------------------

// Creates an empty NIL-like node. Children are set to NIL.
// Used only to construct the static NIL instance.
template <class T, int N>
NTree<T, N>::NTree() : parent_(nullptr)
{
    for (int i = 0; i < N; ++i)
        nodes[i] = &NTree<T, N>::NIL; // Every child is empty by default
}

// -------------------- Public constructor --------------------

// Initializes node with a key and sets all children to NIL.
template <class T, int N>
NTree<T, N>::NTree(const T &aKey) : key(aKey), parent_(nullptr)
{
    for (int i = 0; i < N; ++i)
        nodes[i] = &NTree<T, N>::NIL; // No children yet
}

// -------------------- Destructor --------------------

// Deletes all child subtrees, but only if they are not NIL.
// The NIL children are not deleted, because they are static and shared.
template <class T, int N>
NTree<T, N>::~NTree()
{
    for (int i = 0; i < N; ++i)
    {
        if (nodes[i] != &NTree<T, N>::NIL)
            delete nodes[i]; // Recursively deletes entire subtree
    }
}

// -------------------- Basic accessors --------------------

// A node is empty only if it *is* the static NIL instance.
// Pointer comparison is used, not key or children.
template <class T, int N>
bool NTree<T, N>::isEmpty() const
{
    return this == &NTree<T, N>::NIL;
}

// Return the stored key by const reference. Throws if NIL is accessed.
template <class T, int N>
const T &NTree<T, N>::getKey() const
{
    if (isEmpty())
        throw std::domain_error("Attempting to access key of empty NTree (NIL).");
    return key;
}

// Mutable version of getKey()
template <class T, int N>
T &NTree<T, N>::getKey()
{
    if (isEmpty())
        throw std::domain_error("Attempting to access key of empty NTree (NIL).");
    return key;
}

// -------------------- Child access --------------------

// Access child at given index; returns reference to the subtree root.
// Throws for NIL node or index out of bounds.
template <class T, int N>
NTree<T, N> &NTree<T, N>::operator[](int index) const
{
    if (isEmpty())
        throw std::domain_error("operator[] called on NIL NTree.");
    if (index < 0 || index >= N)
        throw std::out_of_range("Child index out of range.");
    return *nodes[index];
}

// -------------------- Attach / detach children --------------------

// Attaches `tree` as a child at index. The index must be empty (NIL).
// The child's parent pointer is updated to this node.
template <class T, int N>
void NTree<T, N>::attachNTree(int index, NTree<T, N> *tree)
{
    if (isEmpty())
        throw std::domain_error("attachNTree called on empty (NIL) NTree.");
    if (index < 0 || index >= N)
        throw std::out_of_range("Child index out of range.");
    if (nodes[index] != &NTree<T, N>::NIL)
        throw std::domain_error("Attempting to overwrite a non-empty subtree.");

    nodes[index] = tree;           // Install child
    if (tree != &NTree<T, N>::NIL) // NIL's parent must never be changed
        tree->parent_ = this;      // Update parent pointer
}

// Detaches child at index and returns it. The pointer is not deleted.
// Child’s parent pointer is cleared.
template <class T, int N>
NTree<T, N> *NTree<T, N>::detachNTree(int index)
{
    if (isEmpty())
        throw std::domain_error("detachNTree called on empty (NIL) NTree.");
    if (index < 0 || index >= N)
        throw std::out_of_range("Child index out of range.");

    NTree<T, N> *detached = nodes[index]; // Save pointer to return
    nodes[index] = &NTree<T, N>::NIL;     // Replace with NIL

    if (detached != &NTree<T, N>::NIL)
        detached->parent_ = nullptr; // Detached subtree becomes root

    return detached;
}

// -------------------- Size and search functions --------------------

// Recursively counts this node (1) and the size of every subtree.
// NIL returns size 0.
template <class T, int N>
int NTree<T, N>::getSize() const
{
    if (isEmpty())
        return 0;

    int size = 1; // Count this node
    for (int i = 0; i < N; ++i)
        size += nodes[i]->getSize();
    return size;
}

// Recursively searches for the given key by depth-first traversal.
// Returns pointer to node or nullptr if not found.
template <class T, int N>
NTree<T, N> *NTree<T, N>::search(const T &aKey) const
{
    if (isEmpty())
        return nullptr;

    if (key == aKey)
        return const_cast<NTree<T, N> *>(this); // Remove const for return

    // Search each child subtree
    for (int i = 0; i < N; ++i)
    {
        NTree<T, N> *result = nodes[i]->search(aKey);
        if (result)
            return result;
    }
    return nullptr;
}

// -------------------- Level-order traversal (non-const) --------------------

// Performs BFS traversal using a minimal internal queue implementation.
template <class T, int N>
void NTree<T, N>::traverseLevelOrder(std::function<void(T &)> visitor)
{
    if (isEmpty())
        return;

    // Internal minimal queue type storing tree-node pointers
    struct SimpleQueue
    {
        struct QNode
        {
            NTree<T, N> *node; // Stored tree node pointer
            QNode *next;       // Next queue node
            QNode(NTree<T, N> *n) : node(n), next(nullptr) {}
        };

        QNode *head = nullptr; // First element in queue
        QNode *tail = nullptr; // Last element in queue

        // Add node pointer to queue tail
        void push(NTree<T, N> *n)
        {
            if (n == &NTree<T, N>::NIL)
                return; // Never add NIL nodes to queue

            QNode *q = new QNode(n);
            if (!tail)
            {
                head = tail = q;
            }
            else
            {
                tail->next = q;
                tail = q;
            }
        }

        // Remove and return node pointer from queue head
        NTree<T, N> *pop()
        {
            if (!head)
                return nullptr;

            QNode *q = head;
            NTree<T, N> *n = q->node;

            head = head->next;
            if (!head)
                tail = nullptr; // Queue became empty

            delete q; // Free queue wrapper node
            return n;
        }

        bool empty() const { return head == nullptr; }
    };

    SimpleQueue queue;
    queue.push(this); // Start BFS at current node

    while (!queue.empty())
    {
        NTree<T, N> *current = queue.pop();
        if (!current || current->isEmpty())
            continue; // Skip NIL or invalid

        visitor(current->key); // Visit node's stored value

        // Add each non-NIL child
        for (int i = 0; i < N; ++i)
        {
            if (current->nodes[i] != &NTree<T, N>::NIL)
                queue.push(current->nodes[i]);
        }
    }
}

// -------------------- Level-order traversal (const version) --------------------

// Identical to non-const traversal, except visitor receives const T&.
template <class T, int N>
void NTree<T, N>::traverseLevelOrder(std::function<void(const T &)> visitor) const
{
    if (isEmpty())
        return;

    // Minimal queue storing const node pointers
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
            {
                head = tail = q;
            }
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
    };

    SimpleQueue queue;
    queue.push(this);

    while (!queue.empty())
    {
        const NTree<T, N> *current = queue.pop();
        if (!current || current->isEmpty())
            continue;

        visitor(current->key); // Visit this node's data

        for (int i = 0; i < N; ++i)
        {
            if (current->nodes[i] != &NTree<T, N>::NIL)
                queue.push(current->nodes[i]);
        }
    }
}