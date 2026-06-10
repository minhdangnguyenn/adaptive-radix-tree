#include "../catch/catch.hpp"

#include "../include/ART.h"

TEST_CASE("TEST BASIC NODE4", "[art_basic_nodes]") {
    Node4 node4;
    uint8_t keys[4] = {0x01, 0x05, 0x10, 0x1b};
    Node4 *ptrs[4] = {new Node4(), new Node4(), new Node4(), new Node4()};

    memcpy(node4.keys, keys, 4);
    memcpy(node4.children, ptrs, 4 * sizeof(Node *));
    node4.child_num = 4;

    REQUIRE(node4.findChild(0x01) == &node4.children[0]);
    REQUIRE(node4.findChild(0x05) == &node4.children[1]);
    REQUIRE(node4.findChild(0x10) == &node4.children[2]);
    REQUIRE(node4.findChild(0x1b) == &node4.children[3]);
    REQUIRE(node4.findChild(0x1a) == nullptr);
}

TEST_CASE("TEST BASIC NODE256", "[art_basic_nodes]") {
    Node256 node256;
    Node256 *ptrs[4] = {new Node256(), new Node256(), new Node256(),
                        new Node256()};

    memcpy(node256.children, ptrs, 4 * sizeof(Node *));
    node256.child_num = 4;

    REQUIRE(node256.findChild(0x00) == &node256.children[0]);
    REQUIRE(node256.findChild(0x01) == &node256.children[1]);
    REQUIRE(node256.findChild(0x02) == &node256.children[2]);
    REQUIRE(node256.findChild(0x03) == &node256.children[3]);
    // Node256 wont return nullptr
    REQUIRE(node256.findChild(0x04) == &node256.children[4]);
}

TEST_CASE("TEST BASIC ADD CHILD NODE4", "[art_basic_nodes]") {
    Node4 *innernode = new Node4();
    Node4 *childnodes[5] = {new Node4(), new Node4(), new Node4(), new Node4(),
                            new Node4()};
    Node **ptr_from_parent = reinterpret_cast<Node **>(&innernode);
    innernode->addChild(ptr_from_parent, 0x5, childnodes[0]);
    innernode->addChild(ptr_from_parent, 0x4, childnodes[1]);
    innernode->addChild(ptr_from_parent, 0x6, childnodes[2]);
    innernode->addChild(ptr_from_parent, 0x7, childnodes[3]);

    REQUIRE(innernode->keys[0] == 0x4);
    REQUIRE(innernode->keys[1] == 0x5);
    REQUIRE(innernode->keys[2] == 0x6);
    REQUIRE(innernode->keys[3] == 0x7);

    REQUIRE(innernode->children[0] == childnodes[1]);
    REQUIRE(innernode->children[1] == childnodes[0]);
    REQUIRE(innernode->children[2] == childnodes[2]);
    REQUIRE(innernode->children[3] == childnodes[3]);
    REQUIRE((*ptr_from_parent)->get_type() == NodeTypes::N4);

    innernode->addChild(ptr_from_parent, 0x8, childnodes[4]);
    REQUIRE((*ptr_from_parent)->get_type() == NodeTypes::N16);
    REQUIRE(((Node16 *)(*ptr_from_parent))->children[4] == childnodes[4]);
    REQUIRE(*((*ptr_from_parent)->findChild(0x8)) == childnodes[4]);

    delete *ptr_from_parent;
}

TEST_CASE("ART INSERT AND LOOKUP KEY- LEAF ONLY", "[art_basic]") {
    Key keys[20];
    ART *index = new ART(keys, 20);

    keys[15].setInt(7);
    REQUIRE(index->insert(keys[15], 15));

    Key lkupKey;
    lkupKey.setInt(7);
    REQUIRE(index->lookup(lkupKey) == 15);
    delete index;
}

TEST_CASE("ART INSERT AND LOOKUP KEY- MORE KEYS", "[art_basic]") {
    Key keys[230];
    ART *index = new ART(keys, 230);

    // Generate keys
    for (uint64_t i = 0; i < 230; i++) {
        keys[i].setInt(i + 1);
        REQUIRE(index->insert(keys[i], i));
    }

    Key lkupKey;
    lkupKey.setInt(8);
    REQUIRE(index->lookup(lkupKey) == 7);
    lkupKey.setInt(250);
    REQUIRE(index->lookup(lkupKey) == (uint64_t)-1);
    delete index;
}
