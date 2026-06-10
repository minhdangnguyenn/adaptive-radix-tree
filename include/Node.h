#pragma once

#include "Key.h"

enum class NodeTypes : uint8_t { N4 = 0, N16 = 1, N48 = 2, N256 = 3 };

class Node {
  private:
    // the type of node (N4,N16,N48,N256)
    NodeTypes type_;

  public:
    static const unsigned MAX_PREFIX_LENGTH = 9;
    Node(NodeTypes type) : type_(type), child_num(0), prefix_len(0) {}
    virtual ~Node() {}
    NodeTypes get_type() { return type_; }

    /**
     * Copy prefix from other Node.
     */
    void copyPrefix(Node *other) {
        prefix_len = other->prefix_len;
        memcpy(prefix, other->prefix, std::min(prefix_len, MAX_PREFIX_LENGTH));
    }

    // the number of children
    uint16_t child_num;
    // the prefix length
    uint32_t prefix_len;
    // the prefix
    uint8_t prefix[MAX_PREFIX_LENGTH];
    /**
     * Return the address of the pointer to the child, if found, else nullptr.
     */
    virtual Node **findChild(uint8_t keyByte) = 0;
    /**
     * Add the given child pointer to this node. If necessary, grow and update
     * ptr_in_parent.
     */
    virtual void addChild(Node **ptr_in_parent, uint8_t keyByte,
                          Node *child) = 0;

    /**
     * Static method to test if a node pointer is actually a value
     */
    static bool isLeaf(Node *node) {
        return reinterpret_cast<uint64_t>(node) & 1;
    }
    /**
     * Static method to create a Node*, which is actually a leaf with value
     */
    static Node *makeLeaf(uint64_t value) {
        return reinterpret_cast<Node *>((value << 1) | 1);
    }
    /**
     * Static method to get a value from a Node* that is actually a leaf
     */
    static uint64_t getLeafValue(Node *node) {
        return reinterpret_cast<uint64_t>(node) >> 1;
    }
};

class Node4 : public Node {
  public:
    uint8_t keys[4];
    Node *children[4];
    Node4() : Node(NodeTypes::N4) {
        memset(keys, 0, sizeof(keys));
        memset(children, 0, sizeof(children));
    }
    ~Node4() {
        for (uint32_t i = 0; i < 4; ++i)
            if (children[i] != nullptr && !Node::isLeaf(children[i]))
                delete children[i];
    }
    Node **findChild(uint8_t keyByte);
    void addChild(Node **ptr_in_parent, uint8_t keyByte, Node *child);
};

class Node16 : public Node {
  public:
    uint8_t keys[16];
    Node *children[16];
    Node16() : Node(NodeTypes::N16) {
        memset(keys, 0, sizeof(keys));
        memset(children, 0, sizeof(children));
    }
    ~Node16() {
        for (uint32_t i = 0; i < 16; ++i)
            if (children[i] != nullptr && !Node::isLeaf(children[i]))
                delete children[i];
    }
    Node **findChild(uint8_t keyByte);
    void addChild(Node **ptr_in_parent, uint8_t keyByte, Node *child);
};

class Node48 : public Node {
  public:
    uint8_t child_index[256];
    Node *children[48];
    inline static uint8_t EMPTY = 48;
    Node48() : Node(NodeTypes::N48) {
        memset(child_index, EMPTY, sizeof(child_index));
        memset(children, 0, sizeof(children));
    }
    ~Node48() {
        for (uint32_t i = 0; i < 48; ++i)
            if (children[i] != nullptr && !Node::isLeaf(children[i]))
                delete children[i];
    }
    Node **findChild(uint8_t keyByte);
    void addChild(Node **ptr_in_parent, uint8_t keyByte, Node *child);
};

class Node256 : public Node {
  public:
    Node *children[256];
    Node256() : Node(NodeTypes::N256) { memset(children, 0, sizeof(children)); }
    ~Node256() {
        for (uint32_t i = 0; i < 256; ++i)
            if (children[i] != nullptr && !Node::isLeaf(children[i]))
                delete children[i];
    }
    Node **findChild(uint8_t keyByte);
    void addChild(Node **ptr_in_parent, uint8_t keyByte, Node *child);
};
