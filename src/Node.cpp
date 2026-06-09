#include "../include/Node.h"
#include <smmintrin.h>

Node **Node4::findChild(uint8_t keyByte) {
    if (child_num > 0 && keys[0] == keyByte)
        return &children[0];
    if (child_num > 1 && keys[1] == keyByte)
        return &children[1];
    if (child_num > 2 && keys[2] == keyByte)
        return &children[2];
    return child_num > 3 && keys[3] == keyByte ? &children[3] : nullptr;
}

void Node4::addChild(Node **ptr_in_parent, uint8_t keyByte, Node *child) {
    if (this->child_num < 4) {
        int pos = child_num;
        while (pos > 0 && keys[pos - 1] > keyByte)
            pos--;

        memmove(keys + pos + 1, keys + pos, child_num - pos);
        memmove(children + pos + 1, children + pos,
                (child_num - pos) * sizeof(Node *));
        keys[pos] = keyByte;
        children[pos] = child;
        child_num++;
    }
    // else node4 becomes node16
    else {
        Node16 *nn16 = new Node16();
        nn16->copyPrefix(this);
        memcpy(nn16->keys, this->keys, 4);
        memcpy(nn16->children, this->children, 4 * sizeof(Node *));

        nn16->child_num = 4;
        nn16->next_pos = 4;
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
        return &children[__builtin_ctz(bitfield)];
    return nullptr;
}

void Node16::addChild(Node **ptr_in_parent, uint8_t keyByte, Node *child) {
    if (this->child_num < 16) {
        keys[this->next_pos] = keyByte;
        children[this->next_pos] = child;
        this->next_pos++;
        child_num++;
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
        nn48->next_pos = 16;
        nn48->addChild(ptr_in_parent, keyByte, child);
        *ptr_in_parent = nn48;
        memset(this->children, 0, sizeof(this->children));
        delete this;
    }
}

Node **Node48::findChild(uint8_t keyByte) {
    uint8_t index = this->child_index[keyByte];
    if (index != EMPTY)
        return &children[index];

    return nullptr;
}

void Node48::addChild(Node **ptr_in_parent, uint8_t keyByte, Node *child) {
    if (this->child_num < 48) {
        this->child_index[keyByte] = this->next_pos;
        this->children[this->next_pos] = child;
        this->next_pos++;
        child_num++;
    } else {
        Node256 *nn256 = new Node256();
        nn256->copyPrefix(this);
        for (int i = 0; i < 256; i++) {
            uint8_t slot = this->child_index[i];
            if (slot != EMPTY)
                nn256->children[i] = this->children[slot];
        }
        nn256->child_num = this->child_num;
        nn256->addChild(ptr_in_parent, keyByte, child);
        *ptr_in_parent = nn256;
        memset(this->children, 0, sizeof(this->children));
        delete this;
    }
}

Node **Node256::findChild(uint8_t keyByte) { return &children[keyByte]; }

void Node256::addChild(Node **ptr_in_parent, uint8_t keyByte, Node *child) {
    this->child_num += (this->children[keyByte] == nullptr);
    this->children[keyByte] = child;
}
