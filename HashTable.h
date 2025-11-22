#pragma once

#include <functional> // std::hash for hashing keys
#include <stdexcept>  // std::runtime_error for error reporting
#include <memory>     // std::unique_ptr for returning pairs

// Generic templated hash table implementation using separate chaining.
//
// K = key type
// V = value type
//
// Collisions are handled by a linked list (Node) per bucket.
// Table automatically resizes when load factor exceeds loadFactorLimit.
template <typename K, typename V>
class HashTable
{
private:
    // Node represents one key-value pair stored in a bucket's linked list.
    struct Node
    {
        K key;      // Stored key
        V value;    // Associated value for this key
        Node *next; // Pointer to next node in this bucket chain (nullptr if end)

        // Construct node by copying key and value
        Node(const K &k, const V &v) : key(k), value(v), next(nullptr) {}

        // Construct node by copying key and moving value
        Node(const K &k, V &&v) : key(k), value(std::move(v)), next(nullptr) {}
    };

    Node **table;          // Dynamic array of pointers to bucket heads
    size_t capacity;       // Number of buckets in the table
    size_t count;          // Number of stored key-value pairs
    float loadFactorLimit; // Maximum allowed load factor before resizing

    // Compute the bucket index for a given key using std::hash.
    size_t hash(const K &key) const;

    // Double the capacity and re-insert all existing nodes into the new table.
    void resize();

public:
    // Construct hash table with initial bucket count and desired load factor.
    explicit HashTable(size_t initialCapacity = 16, float loadFactor = 0.75f);

    // Destructor releases all nodes and the bucket array.
    ~HashTable();

    // Copy constructor performs a deep copy of all buckets and nodes.
    HashTable(const HashTable &other);

    // Copy assignment operator, also deep copies from other.
    HashTable &operator=(const HashTable &other);

    // Insert or update a key-value pair using copy semantics for value.
    void insert(const K &key, const V &value);

    // Insert or update a key-value pair using move semantics for value.
    void insert(const K &key, V &&value);

    // Get a mutable reference to value for a given key. Throws if key is missing.
    V &get(const K &key);

    // Get a const reference to value for a given key. Throws if key is missing.
    const V &get(const K &key) const;

    // True if key exists in table, false otherwise.
    bool contains(const K &key) const;

    // Remove a key-value pair by key if it exists. Returns true if removed.
    bool remove(const K &key);

    // Delete all key-value pairs and reset all buckets to empty.
    void clear();

    // Number of key-value pairs currently stored.
    size_t size() const;

    // True if there are no stored pairs.
    bool isEmpty() const;

    // Access value by key, creating a default-constructed value if missing.
    V &operator[](const K &key);

    // Total number of buckets in the underlying table.
    size_t bucketCount() const;

    // Current load factor = count / capacity.
    float loadFactor() const;

    // Return the bucket index that currently stores the given key.
    // Throws if the key is not found.
    size_t bucketIndex(const K &key) const;

    // Count how many buckets have at least one node.
    size_t nonEmptyBucketCount() const;

    // Compute maximum chain length (longest linked list) across all buckets.
    size_t longestChainLength() const;
};

// -------------------- Implementation --------------------

// Compute hash index for a given key by calling std::hash and modding capacity.
template <typename K, typename V>
size_t HashTable<K, V>::hash(const K &key) const
{
    std::hash<K> hasher;           // Standard hash functor for key type
    return hasher(key) % capacity; // Constrain hash into [0, capacity)
}

// Resize table by doubling capacity and rehashing all existing nodes.
template <typename K, typename V>
void HashTable<K, V>::resize()
{
    size_t oldCapacity = capacity; // Store old bucket count
    Node **oldTable = table;       // Keep pointer to old bucket array

    capacity *= 2;                // Double number of buckets
    table = new Node *[capacity]; // Allocate new bucket array
    for (size_t i = 0; i < capacity; ++i)
    {
        table[i] = nullptr; // Initialize all buckets as empty
    }

    count = 0; // Will be recomputed as we re-insert nodes

    // Re-insert each key-value pair from old table into new table.
    // Note: this recomputes hash indices based on new capacity.
    for (size_t i = 0; i < oldCapacity; ++i)
    {
        Node *current = oldTable[i];
        while (current != nullptr)
        {
            // Insert using move to avoid unnecessary copies of value.
            insert(current->key, std::move(current->value));

            Node *temp = current;    // Save pointer to node to delete
            current = current->next; // Advance before deleting
            delete temp;             // Free old node
        }
    }

    delete[] oldTable; // Free the old bucket array
}

// Construct hash table with given capacity and load factor limit.
template <typename K, typename V>
HashTable<K, V>::HashTable(size_t initialCapacity, float loadFactor)
    : capacity(initialCapacity),
      count(0),
      loadFactorLimit(loadFactor)
{
    table = new Node *[capacity]; // Allocate array of bucket pointers
    for (size_t i = 0; i < capacity; ++i)
    {
        table[i] = nullptr; // All buckets start empty
    }
}

// Destructor releases all stored nodes and the bucket array.
template <typename K, typename V>
HashTable<K, V>::~HashTable()
{
    clear();        // Delete all nodes in all buckets
    delete[] table; // Delete bucket array
}

// Copy constructor: deep copy of another HashTable.
template <typename K, typename V>
HashTable<K, V>::HashTable(const HashTable &other)
    : capacity(other.capacity),
      count(0), // Will be updated via insert()
      loadFactorLimit(other.loadFactorLimit)
{
    table = new Node *[capacity]; // New bucket array
    for (size_t i = 0; i < capacity; ++i)
    {
        table[i] = nullptr; // Initialize buckets as empty
    }

    // Copy all key-value pairs by inserting them into this table.
    for (size_t i = 0; i < other.capacity; ++i)
    {
        Node *current = other.table[i];
        while (current != nullptr)
        {
            insert(current->key, current->value); // Use copy insert
            current = current->next;
        }
    }
}

// Copy assignment: clear current contents, then deep copy from other.
template <typename K, typename V>
HashTable<K, V> &HashTable<K, V>::operator=(const HashTable &other)
{
    if (this != &other) // Protect against self-assignment
    {
        clear();        // Remove all existing nodes
        delete[] table; // Free existing bucket array

        capacity = other.capacity;
        count = 0; // Recomputed as we insert
        loadFactorLimit = other.loadFactorLimit;

        table = new Node *[capacity]; // Allocate new buckets
        for (size_t i = 0; i < capacity; ++i)
        {
            table[i] = nullptr;
        }

        // Insert all key-value pairs from other
        for (size_t i = 0; i < other.capacity; ++i)
        {
            Node *current = other.table[i];
            while (current != nullptr)
            {
                insert(current->key, current->value);
                current = current->next;
            }
        }
    }
    return *this;
}

// Insert or update key-value pair using copy semantics for value.
template <typename K, typename V>
void HashTable<K, V>::insert(const K &key, const V &value)
{
    size_t index = hash(key); // Compute bucket index
    Node *current = table[index];

    // Traverse chain to find existing key
    while (current != nullptr)
    {
        if (current->key == key)
        {
            // Key already exists: overwrite value and return
            current->value = value;
            return;
        }
        current = current->next;
    }

    // Key not found: create new node and insert at bucket head
    Node *newNode = new Node(key, value);
    newNode->next = table[index]; // Link existing head after new node
    table[index] = newNode;       // New node becomes bucket head
    ++count;

    // Check current load factor and resize if limit exceeded
    if (static_cast<float>(count) / static_cast<float>(capacity) > loadFactorLimit)
    {
        resize();
    }
}

// Insert or update key-value pair using move semantics for value.
template <typename K, typename V>
void HashTable<K, V>::insert(const K &key, V &&value)
{
    size_t index = hash(key);
    Node *current = table[index];

    // Look for existing key in the chain
    while (current != nullptr)
    {
        if (current->key == key)
        {
            // Overwrite existing value by move
            current->value = std::move(value);
            return;
        }
        current = current->next;
    }

    // Insert a new node at head using move to construct value
    Node *newNode = new Node(key, std::move(value));
    newNode->next = table[index];
    table[index] = newNode;
    ++count;

    // Resize if load factor threshold exceeded
    if (static_cast<float>(count) / static_cast<float>(capacity) > loadFactorLimit)
    {
        resize();
    }
}

// Get mutable reference to value for key. Throws if key not present.
template <typename K, typename V>
V &HashTable<K, V>::get(const K &key)
{
    size_t index = hash(key);
    Node *current = table[index];

    // Traverse bucket chain to find matching key
    while (current != nullptr)
    {
        if (current->key == key)
        {
            return current->value; // Return reference to stored value
        }
        current = current->next;
    }

    // If we reach here, key was not found
    throw std::runtime_error("Key not found in hash table");
}

// Const version of get() for read-only tables.
template <typename K, typename V>
const V &HashTable<K, V>::get(const K &key) const
{
    size_t index = hash(key);
    Node *current = table[index];

    while (current != nullptr)
    {
        if (current->key == key)
        {
            return current->value;
        }
        current = current->next;
    }

    throw std::runtime_error("Key not found in hash table");
}

// Check if key exists in table by scanning the appropriate bucket chain.
template <typename K, typename V>
bool HashTable<K, V>::contains(const K &key) const
{
    size_t index = hash(key);
    Node *current = table[index];

    while (current != nullptr)
    {
        if (current->key == key)
        {
            return true;
        }
        current = current->next;
    }

    return false;
}

// Remove key-value pair from table if present. Returns true if removal happened.
template <typename K, typename V>
bool HashTable<K, V>::remove(const K &key)
{
    size_t index = hash(key);
    Node *current = table[index];
    Node *prev = nullptr;

    // Traverse chain while tracking previous pointer
    while (current != nullptr)
    {
        if (current->key == key)
        {
            // Adjust bucket head or previous->next to skip current
            if (prev == nullptr)
            {
                table[index] = current->next; // Removing head of bucket
            }
            else
            {
                prev->next = current->next; // Removing node in middle/end
            }

            delete current; // Free removed node
            --count;
            return true;
        }

        prev = current;
        current = current->next;
    }

    // Key not found
    return false;
}

// Delete all nodes from all buckets and reset count to zero.
template <typename K, typename V>
void HashTable<K, V>::clear()
{
    for (size_t i = 0; i < capacity; ++i)
    {
        Node *current = table[i];
        while (current != nullptr)
        {
            Node *temp = current;
            current = current->next;
            delete temp; // Free node
        }
        table[i] = nullptr; // Bucket now empty
    }

    count = 0;
}

// Return how many key-value pairs are stored.
template <typename K, typename V>
size_t HashTable<K, V>::size() const
{
    return count;
}

// True if no key-value pairs stored.
template <typename K, typename V>
bool HashTable<K, V>::isEmpty() const
{
    return count == 0;
}

// Access value for key, inserting a default-constructed value if key not present.
template <typename K, typename V>
V &HashTable<K, V>::operator[](const K &key)
{
    if (!contains(key))
    {
        // Insert a new key with default-constructed value
        insert(key, V());
    }
    return get(key); // Now guaranteed to exist
}

// Number of buckets allocated in this hash table.
template <typename K, typename V>
size_t HashTable<K, V>::bucketCount() const
{
    return capacity;
}

// Compute current load factor (ratio of entries to buckets).
template <typename K, typename V>
float HashTable<K, V>::loadFactor() const
{
    if (capacity == 0)
        return 0.0f;

    return static_cast<float>(count) / static_cast<float>(capacity);
}

// Return bucket index where key currently lives; throws if key not present.
template <typename K, typename V>
size_t HashTable<K, V>::bucketIndex(const K &key) const
{
    size_t index = hash(key);
    Node *current = table[index];

    while (current != nullptr)
    {
        if (current->key == key)
        {
            return index; // We found the key in this bucket
        }
        current = current->next;
    }

    throw std::runtime_error("Key not found in hash table");
}

// Count how many buckets are non-empty (contain at least one node).
template <typename K, typename V>
size_t HashTable<K, V>::nonEmptyBucketCount() const
{
    size_t used = 0;

    for (size_t i = 0; i < capacity; ++i)
    {
        if (table[i] != nullptr)
        {
            ++used;
        }
    }

    return used;
}

// Compute the maximum length of any bucket's linked-list chain.
template <typename K, typename V>
size_t HashTable<K, V>::longestChainLength() const
{
    size_t maxLength = 0;

    for (size_t i = 0; i < capacity; ++i)
    {
        size_t length = 0;
        Node *current = table[i];

        while (current != nullptr)
        {
            ++length;
            current = current->next;
        }

        if (length > maxLength)
        {
            maxLength = length;
        }
    }

    return maxLength;
}