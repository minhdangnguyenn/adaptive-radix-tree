#include "../include/Node.h"
#include <smmintrin.h>

Node **Node4::findChild(uint8_t keyByte) {
    if (this->child_num > 0 && keys[0] == keyByte)
        return &children[0];
    else if (this->child_num > 1 && keys[1] == keyByte)
        return &children[1];
    else if (this->child_num > 2 && keys[2] == keyByte)
        return &children[2];
    return this->child_num > 3 && keys[3] == keyByte ? &this->children[3]
                                                     : nullptr;
}

void Node4::addChild(Node **ptr_in_parent, uint8_t keyByte, Node *child) {
    if (this->child_num < 4) {
        int pos = this->child_num;
        while (pos > 0 && keys[pos - 1] > keyByte)
            pos--;

        memmove(this->keys + pos + 1, this->keys + pos, this->child_num - pos);
        memmove(this->children + pos + 1, this->children + pos,
                (this->child_num - pos) * sizeof(Node *));
        this->keys[pos] = keyByte;
        this->children[pos] = child;
        this->child_num++;
    }
    // else node4 becomes node16
    else {
        Node16 *nn16 = new Node16();
        nn16->copyPrefix(this);
        memcpy(nn16->keys, this->keys, 4);
        memcpy(nn16->children, this->children, 4 * sizeof(Node *));

        nn16->child_num = 4;
        nn16->addChild(ptr_in_parent, keyByte, child);

        *ptr_in_parent = nn16;

        memset(this->children, 0, sizeof(this->children));
        delete this;
    }
}

/**
 * as described in the ART paper
 * use SIMD to find the child pointer corresponding to keyByte
 * return nullptr if not found
 */
Node **Node16::findChild(uint8_t keyByte) {
    __m128i key = _mm_set1_epi8(keyByte);
    __m128i cmp = _mm_cmpeq_epi8(
        key, _mm_loadu_si128(reinterpret_cast<const __m128i *>(keys)));

    uint16_t mask = (1u << child_num) - 1;
    uint16_t bitfield = _mm_movemask_epi8(cmp) & mask;

    if (bitfield)
        return &this->children[__builtin_ctz(bitfield)];
    return nullptr;
}

void Node16::addChild(Node **ptr_in_parent, uint8_t keyByte, Node *child) {
    if (this->child_num < 16) {
        this->keys[this->child_num] = keyByte;
        this->children[this->child_num] = child;
        this->child_num++;
    }
    // else node16 becomes node48
    else {
        Node48 *nn48 = new Node48();
        nn48->copyPrefix(this);
        for (int i = 0; i < 16; i++) {
            uint8_t key = this->keys[i];
            nn48->child_index[key] = i;
            nn48->children[i] = this->children[i];
        }
        nn48->child_num = 16;
        nn48->addChild(ptr_in_parent, keyByte, child);
        *ptr_in_parent = nn48;
        memset(this->children, 0, sizeof(this->children));
        delete this;
    }
}

Node **Node48::findChild(uint8_t keyByte) {
    uint8_t index = this->child_index[keyByte];
    if (index != EMPTY)
        return &this->children[index];

    return nullptr;
}

void Node48::addChild(Node **ptr_in_parent, uint8_t keyByte, Node *child) {
    if (this->child_num < 48) {
        this->child_index[keyByte] = this->child_num;
        this->children[child_num] = child;
        this->child_num++;
    } else {
        Node256 *nn256 = new Node256();
        nn256->copyPrefix(this);
        for (int i = 0; i < 256; i++) {
            uint8_t slot = this->child_index[i];
            if (slot != EMPTY)
                nn256->children[i] = this->children[slot];
        }
        nn256->child_num = child_num;
        nn256->addChild(ptr_in_parent, keyByte, child);
        *ptr_in_parent = nn256;
        memset(this->children, 0, sizeof(this->children));
        delete this;
    }
}

Node **Node256::findChild(uint8_t keyByte) { return &this->children[keyByte]; }

void Node256::addChild(Node **ptr_in_parent, uint8_t keyByte, Node *child) {
    this->child_num += (this->children[keyByte] == nullptr);
    this->children[keyByte] = child;
}

