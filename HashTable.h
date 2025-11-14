#pragma once

#include <functional>
#include <stdexcept>
#include <memory>

// Generic templated hash table implementation with chaining
template <typename K, typename V>
class HashTable
{
private:
    // Node representing a single key-value pair in a linked list
    struct Node
    {
        K key;      // Key stored in this node
        V value;    // Associated value
        Node *next; // Pointer to next node in the same bucket

        Node(const K &k, const V &v) : key(k), value(v), next(nullptr) {}
        Node(const K &k, V &&v) : key(k), value(std::move(v)), next(nullptr) {}
    };

    Node **table;          // Array of pointers to linked lists (buckets)
    size_t capacity;       // Number of buckets
    size_t count;          // Number of key-value pairs
    float loadFactorLimit; // Load factor threshold for resizing

    // Computes hash index for a given key
    size_t hash(const K &key) const;

    // Doubles the table size and rehashes all elements
    void resize();

public:
    // Constructor initializes table with given capacity and load factor
    explicit HashTable(size_t initialCapacity = 16, float loadFactor = 0.75f);

    // Destructor releases all memory
    ~HashTable();

    // Copy constructor (deep copy)
    HashTable(const HashTable &other);

    // Copy assignment operator (deep copy)
    HashTable &operator=(const HashTable &other);

    // Inserts or updates a key-value pair (copy version)
    void insert(const K &key, const V &value);

    // Inserts or updates a key-value pair (move version)
    void insert(const K &key, V &&value);

    // Retrieves a value by key (throws if not found)
    V &get(const K &key);

    // Retrieves a const reference to a value by key (throws if not found)
    const V &get(const K &key) const;

    // Retrieves a value by key or returns default if key not found
    V getOrDefault(const K &key, const V &defaultValue) const;

    // Checks if a key exists in the hash table
    bool contains(const K &key) const;

    // Removes a key-value pair if it exists
    bool remove(const K &key);

    // Clears all elements from the table
    void clear();

    // Returns total number of key-value pairs
    size_t size() const;

    // Checks if table is empty
    bool isEmpty() const;

    // Accesses or creates a key-value pair using operator[]
    V &operator[](const K &key);

    // Returns a list of all key-value pairs in the table
    std::unique_ptr<std::pair<K, V>[]> getAllPairs(size_t &count) const;

    // Returns total number of buckets allocated in the table
    size_t bucketCount() const;

    // Returns the current load factor (count / capacity)
    float loadFactor() const;

    // Returns the bucket index that currently stores the provided key
    size_t bucketIndex(const K &key) const;

    // Returns how many buckets contain at least one node
    size_t nonEmptyBucketCount() const;

    // Returns the longest linked-list chain among all buckets
    size_t longestChainLength() const;

    // Returns the average chain length across all populated buckets
    double averageChainLength() const;
};

// -----------------------------
// Implementation
// -----------------------------

// Computes hash index for a given key
template <typename K, typename V>
size_t HashTable<K, V>::hash(const K &key) const
{
    std::hash<K> hasher;
    return hasher(key) % capacity;
}

// Resizes table and rehashes all nodes into a larger array
template <typename K, typename V>
void HashTable<K, V>::resize()
{
    size_t oldCapacity = capacity;
    Node **oldTable = table;

    capacity *= 2;
    table = new Node *[capacity];
    for (size_t i = 0; i < capacity; ++i)
    {
        table[i] = nullptr;
    }
    count = 0;

    // Move elements from old table to new table
    for (size_t i = 0; i < oldCapacity; ++i)
    {
        Node *current = oldTable[i];
        while (current != nullptr)
        {
            insert(current->key, std::move(current->value));
            Node *temp = current;
            current = current->next;
            delete temp;
        }
    }

    delete[] oldTable;
}

// Constructor initializes buckets
template <typename K, typename V>
HashTable<K, V>::HashTable(size_t initialCapacity, float loadFactor)
    : capacity(initialCapacity), count(0), loadFactorLimit(loadFactor)
{
    table = new Node *[capacity];
    for (size_t i = 0; i < capacity; ++i)
    {
        table[i] = nullptr;
    }
}

// Destructor cleans up all allocated memory
template <typename K, typename V>
HashTable<K, V>::~HashTable()
{
    clear();
    delete[] table;
}

// Copy constructor for deep cloning another hash table
template <typename K, typename V>
HashTable<K, V>::HashTable(const HashTable &other)
    : capacity(other.capacity), count(0), loadFactorLimit(other.loadFactorLimit)
{
    table = new Node *[capacity];
    for (size_t i = 0; i < capacity; ++i)
    {
        table[i] = nullptr;
    }

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

// Copy assignment operator
template <typename K, typename V>
HashTable<K, V> &HashTable<K, V>::operator=(const HashTable &other)
{
    if (this != &other)
    {
        clear();
        delete[] table;

        capacity = other.capacity;
        count = 0;
        loadFactorLimit = other.loadFactorLimit;

        table = new Node *[capacity];
        for (size_t i = 0; i < capacity; ++i)
        {
            table[i] = nullptr;
        }

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

// Inserts a key-value pair or updates existing one (copy version)
template <typename K, typename V>
void HashTable<K, V>::insert(const K &key, const V &value)
{
    size_t index = hash(key);
    Node *current = table[index];

    // Check for existing key
    while (current != nullptr)
    {
        if (current->key == key)
        {
            current->value = value;
            return;
        }
        current = current->next;
    }

    // Insert new node at bucket head
    Node *newNode = new Node(key, value);
    newNode->next = table[index];
    table[index] = newNode;
    count++;

    // Resize if load factor exceeded
    if (static_cast<float>(count) / capacity > loadFactorLimit)
    {
        resize();
    }
}

// Inserts a key-value pair or updates existing one (move version)
template <typename K, typename V>
void HashTable<K, V>::insert(const K &key, V &&value)
{
    size_t index = hash(key);
    Node *current = table[index];

    // Check for existing key
    while (current != nullptr)
    {
        if (current->key == key)
        {
            current->value = std::move(value);
            return;
        }
        current = current->next;
    }

    // Insert new node using move semantics
    Node *newNode = new Node(key, std::move(value));
    newNode->next = table[index];
    table[index] = newNode;
    count++;

    // Resize if load factor exceeded
    if (static_cast<float>(count) / capacity > loadFactorLimit)
    {
        resize();
    }
}

// Retrieves reference to value by key (throws if not found)
template <typename K, typename V>
V &HashTable<K, V>::get(const K &key)
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

// Const version of get()
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

// Returns value or default if key is missing
template <typename K, typename V>
V HashTable<K, V>::getOrDefault(const K &key, const V &defaultValue) const
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

    return defaultValue;
}

// Checks whether a key exists
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

// Removes a key-value pair if found
template <typename K, typename V>
bool HashTable<K, V>::remove(const K &key)
{
    size_t index = hash(key);
    Node *current = table[index];
    Node *prev = nullptr;

    while (current != nullptr)
    {
        if (current->key == key)
        {
            // Remove node from chain
            if (prev == nullptr)
            {
                table[index] = current->next;
            }
            else
            {
                prev->next = current->next;
            }
            delete current;
            count--;
            return true;
        }
        prev = current;
        current = current->next;
    }

    return false;
}

// Clears all elements from the table
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
            delete temp;
        }
        table[i] = nullptr;
    }
    count = 0;
}

// Returns current number of key-value pairs
template <typename K, typename V>
size_t HashTable<K, V>::size() const
{
    return count;
}

// Checks if table contains no elements
template <typename K, typename V>
bool HashTable<K, V>::isEmpty() const
{
    return count == 0;
}

// Access or create a key-value pair using operator[]
template <typename K, typename V>
V &HashTable<K, V>::operator[](const K &key)
{
    if (!contains(key))
    {
        insert(key, V());
    }
    return get(key);
}

// Returns all key-value pairs as an array of pairs
template <typename K, typename V>
std::unique_ptr<std::pair<K, V>[]> HashTable<K, V>::getAllPairs(size_t &outCount) const
{
    outCount = count;
    if (count == 0)
    {
        return nullptr;
    }

    auto result = std::make_unique<std::pair<K, V>[]>(count);
    size_t index = 0;

    // Copy all pairs into result array
    for (size_t i = 0; i < capacity; ++i)
    {
        Node *current = table[i];
        while (current != nullptr)
        {
            result[index++] = {current->key, current->value};
            current = current->next;
        }
    }

    return result;
}

// Returns number of buckets currently allocated
template <typename K, typename V>
size_t HashTable<K, V>::bucketCount() const
{
    return capacity;
}

// Returns current load factor
template <typename K, typename V>
float HashTable<K, V>::loadFactor() const
{
    if (capacity == 0)
    {
        return 0.0f;
    }
    return static_cast<float>(count) / static_cast<float>(capacity);
}

// Returns bucket index that stores the provided key
template <typename K, typename V>
size_t HashTable<K, V>::bucketIndex(const K &key) const
{
    size_t index = hash(key);
    Node *current = table[index];

    while (current != nullptr)
    {
        if (current->key == key)
        {
            return index;
        }
        current = current->next;
    }

    throw std::runtime_error("Key not found in hash table");
}

// Counts buckets that contain at least one node
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

// Computes the longest linked-list chain length
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

// Computes the average chain length for populated buckets
template <typename K, typename V>
double HashTable<K, V>::averageChainLength() const
{
    size_t usedBuckets = nonEmptyBucketCount();
    if (usedBuckets == 0)
    {
        return 0.0;
    }
    return static_cast<double>(count) / static_cast<double>(usedBuckets);
}
