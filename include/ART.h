#pragma once

#include "Key.h"
#include "Node.h"

class ART {
  private:
    /* data */
    Node *root;

    // use this to go from leaf value to key
    // e.g. val2key[Node::getLeafValue(node)] translates value to key
    Key *val2key;
    uint64_t val2key_len;

  public:
    ART(Key *, uint64_t);
    ~ART();

    // Our test and benchmark framework assumes the following two method
    // signatures

    /*
     * insert - load data into the tree
     * Input: key
     *
     * Return: True if insert was successful, False otherwise
     */
    bool insert(Key &key, uint64_t value);

    /*
     * lookup - search for given key k in data using the index
     * Input: key
     *
     * Return: value, or (uint64_t) -1 if not found
     */
    uint64_t lookup(Key &k);

    /*
     * get_root - returns root node to check space consumption
     * Return: root node
     */
    Node *get_root() { return root; };
    // add additional public member variables and methods below this line if
    // required
    uint32_t checkPrefix(Node *node, Key &key, uint32_t depth);
};
