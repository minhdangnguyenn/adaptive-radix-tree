#include "../include/ART.h"
#include <emmintrin.h>

ART::ART(Key *val2key, uint64_t val2key_len)
    : root(nullptr), val2key(val2key), val2key_len(val2key_len) {}

ART::~ART() {}

const uint64_t NULL_ = (uint64_t)-1;

__attribute__((hot)) uint64_t ART::lookup(Key &k) {
    Node *node = this->root;
    uint32_t depth = 0;
    const uint32_t key_len = k.getKeyLen();

    while (node != nullptr) {
        // I checked every prefix path so I dont need to check the leaf :D
        if (Node::isLeaf(node)) {
            return Node::getLeafValue(node);
        }

        uint32_t next_depth = depth + node->prefix_len;
        if (next_depth >= key_len)
            return NULL_;

        Node **child = node->findChild(k[next_depth]);
        if (child == nullptr || *child == nullptr)
            return NULL_;

        depth = next_depth + 1;
        node = *child;
    }
    return NULL_;
}

__attribute__((hot)) bool ART::insert(Key &k, uint64_t value) {
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
            Key &k2 = this->val2key[Node::getLeafValue(node)]; // loadKey

            uint32_t common_prefix_len = 0;
            uint32_t common_limit = std::min(k.getKeyLen(), k2.getKeyLen());

            while (depth + common_prefix_len < common_limit &&
                   k[depth + common_prefix_len] ==
                       k2[depth + common_prefix_len]) {
                common_prefix_len++;
            }

            nn4->prefix_len = common_prefix_len;
            memcpy(nn4->prefix, &k[depth], common_prefix_len);

            depth = depth + common_prefix_len;
            nn4->addChild(ptr_in_parent, k[depth], new_leaf);
            nn4->addChild(ptr_in_parent, k2[depth], node);
            *ptr_in_parent = nn4;
            return true;
        }

        uint32_t prefix_match = this->checkPrefix(node, k, depth);

        if (prefix_match != node->prefix_len) {
            // prefix mismatch — split inner node, redirect parent
            Node4 *nn4 = new Node4();
            nn4->prefix_len = prefix_match;
            memcpy(nn4->prefix, &k[depth], prefix_match);

            // old node keeps the suffix after the split byte
            // uint32_t new_old_prefix_len = node->prefix_len - prefix_match -
            // 1;
            // I do same as in the paper
            node->prefix_len = node->prefix_len - prefix_match - 1;
            memmove(node->prefix, node->prefix + prefix_match + 1,
                    node->prefix_len);

            *ptr_in_parent = nn4; // = replace() (same as paper)
            nn4->addChild(ptr_in_parent, k[depth + prefix_match], new_leaf);
            nn4->addChild(ptr_in_parent, node->prefix[prefix_match], node);
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
    uint32_t cmp_limit = node->prefix_len;

    uint32_t p = 0;
    while (p < cmp_limit && node->prefix[p] == key[depth + p]) {
        p++;
    }

    if (p == cmp_limit) {
        return node->prefix_len;
    }

    return p;
}
