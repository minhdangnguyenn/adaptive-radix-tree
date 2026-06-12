#include "../include/ART.h"
#include <emmintrin.h>

ART::ART(Key *val2key, uint64_t val2key_len)
    : root(nullptr), val2key(val2key), val2key_len(val2key_len) {}

ART::~ART() {}

const uint64_t NULL_ = (uint64_t)-1;

__attribute__((hot)) uint64_t ART::lookup(Key &k) {
    Node *node = this->root;
    uint32_t depth = 0;

    while (node != nullptr) {
        // I checked every prefix path so I dont need to check the leaf :D
        if (Node::isLeaf(node)) {
            return Node::getLeafValue(node);
        }

        // skip compressed prefix in one jump
        uint32_t next_depth = depth + node->prefix_len;

        // find child for the next key byte
        Node **child = node->findChild(k[next_depth]);
        if (*child == nullptr) {
            return NULL_;
        }

        // descend, advance past prefix and consumed byte
        depth = next_depth + 1;
        node = *child;
    }
    return NULL_;
}

bool ART::insert(Key &k, uint64_t value) {
    Node *new_leaf = Node::makeLeaf(value);
    if (this->root == nullptr) [[unlikely]] {
        // first insert, root becomes leaf
        this->root = new_leaf;
        return true;
    }

    Node *node = this->root;
    Node **ptr_in_parent = &this->root;
    uint32_t depth = 0;

    while (true) {
        if (Node::isLeaf(node)) {
            // hit leaf during descent — expand into inner node with both leaves
            Node4 *nn4 = new Node4();
            Key &k2 = this->val2key[Node::getLeafValue(node)]; // == loadKey()

            uint32_t common_limit = std::min(k.getKeyLen(), k2.getKeyLen());
            while (depth + nn4->prefix_len < common_limit &&
                   k[depth + nn4->prefix_len] == k2[depth + nn4->prefix_len]) {
                nn4->prefix_len++;
            }

            memcpy(nn4->prefix, &k[depth], nn4->prefix_len);

            depth = depth + nn4->prefix_len;
            nn4->addChild(ptr_in_parent, k[depth], new_leaf);
            nn4->addChild(ptr_in_parent, k2[depth], node);
            *ptr_in_parent = nn4;
            return true;
        }

        uint32_t prefix_match_length = this->checkPrefix(node, k, depth);

        if (prefix_match_length != node->prefix_len) {
            // prefix mismatch — split inner node, redirect parent
            Node4 *nn4 = new Node4();
            nn4->prefix_len = prefix_match_length;

            // I do same as in the paper
            memcpy(nn4->prefix, &k[depth], prefix_match_length);

            node->prefix_len = node->prefix_len - prefix_match_length - 1;
            memmove(node->prefix, node->prefix + prefix_match_length + 1,
                    node->prefix_len);

            *ptr_in_parent = nn4; // ( = replace() in pseudo code in paper)
            nn4->addChild(ptr_in_parent, k[depth + prefix_match_length],
                          new_leaf);
            nn4->addChild(ptr_in_parent, node->prefix[prefix_match_length],
                          node);
            return true;
        }

        // prefix matches fully — descend
        depth += node->prefix_len;

        Node **child_ptr = node->findChild(k[depth]);
        Node *child = (child_ptr != nullptr) ? *child_ptr : nullptr;

        if (child != nullptr) {
            // found existing child — continue descent
            ptr_in_parent = child_ptr;
            node = child;
            depth++;
            continue;
        }
        // = else in paper
        // no child for this key byte — add new leaf
        node->addChild(ptr_in_parent, k[depth], new_leaf);
        return true;
    }
}

/**
 * compares the compressed path of a
 * node with the key and returns the number of equal bytes
 */
inline __attribute__((always_inline)) uint32_t
ART::checkPrefix(Node *node, Key &key, uint32_t depth) {
    uint32_t node_prefix_len = node->prefix_len;
    uint32_t cmp_limit = node_prefix_len;

    uint32_t p = 0;
    while (p < cmp_limit && node->prefix[p] == key[depth + p]) {
        p++;
    }

    if (p == cmp_limit) {
        return node_prefix_len;
    }

    return p;
}

