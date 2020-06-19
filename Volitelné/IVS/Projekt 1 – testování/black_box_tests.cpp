//======== Copyright (c) 2017, FIT VUT Brno, All rights reserved. ============//
//
// Purpose:     Red-Black Tree - public interface tests
//
// $NoKeywords: $ivs_project_1 $black_box_tests.cpp
// $Author:     Ondřej Ondryáš <xondry02@stud.fit.vutbr.cz>
// $Date:       $2020-03-03
//============================================================================//
/**
 * @file black_box_tests.cpp
 * @author Ondřej Ondryáš
 * 
 * @brief Implementace testu binarniho stromu.
 */

#include <vector>
#include <algorithm>

#include "gtest/gtest.h"

#include "red_black_tree.h"

class EmptyTree : public ::testing::Test {
protected:
    BinaryTree tree;
};

class NonEmptyTree : public ::testing::Test {
protected:
    void SetUp() override {
        std::vector<std::pair<bool, Node_t *>> out;
        tree.InsertNodes(uniqueValues, out);
        tree.InsertNodes(uniqueValues, out);
    }

    BinaryTree tree;
    const std::vector<int> uniqueValues = {0, 1, 2, 3, 4, 5};
    const int safeValue = 10;
};

class TreeAxioms : public ::testing::Test {
protected:
    void SetUp() override {
        for (int i = 0; i < 1000; i++) {
            tree.InsertNode(i * 10);
        }
    }

    BinaryTree tree;
};

TEST_F(EmptyTree, InsertNode) {
    std::pair<bool, Node_t *> item = tree.InsertNode(1);
    Node_t *itemPtr = item.second;

    EXPECT_TRUE(item.first) << "The item is already present, even though the tree was empty.";
    ASSERT_NE(item.second, nullptr) << "The inserted item pointer must not be null.";
    EXPECT_EQ(item.second->key, 1) << "The inserted item key doesn't match.";

    item = tree.InsertNode(1);
    EXPECT_FALSE(item.first) << "Re-inserting the item yielded a new item result.";
    EXPECT_EQ(item.second, itemPtr) << "Re-inserting the item yielded a pointer to a different node.";
}

TEST_F(EmptyTree, DeleteNode) {
    EXPECT_FALSE(tree.DeleteNode(1)) << "False should be returned when trying to remove a non-existent node.";

    tree.InsertNode(1);
    EXPECT_TRUE(tree.DeleteNode(1)) << "An item that had been added wasn't deleted successfully.";
}

TEST_F(EmptyTree, FindNode) {
    EXPECT_EQ(tree.FindNode(1), nullptr) << "A non-existent node lookup should yield null.";

    std::pair<bool, BinaryTree::Node_t *> item = tree.InsertNode(1);
    Node_t *lookup = tree.FindNode(1);
    EXPECT_EQ(item.second, lookup) << "An item that had been added wasn't found.";
}

TEST_F(EmptyTree, RootNull) {
    EXPECT_EQ(tree.GetRoot(), nullptr) << "The root of an empty tree is not null";

    tree.InsertNode(1);
    EXPECT_NE(tree.GetRoot(), nullptr) << "The root of a non-empty tree is null.";
}

TEST_F(EmptyTree, InsertNodes) {
    std::vector<int> values = {0, 1, 1, 2};
    std::vector<std::pair<bool, Node_t *>> out;
    tree.InsertNodes(values, out);

    bool hasZero = false, hasOne = false, hasOnePtr = false, hasTwo = false;
    Node_t *onePtr = nullptr;

    for (std::pair<bool, Node_t *> item : out) {
        ASSERT_NE(item.second, nullptr) << "The tree has returned a null pointer for one of its nodes.";

        switch (item.second->key) {
            case 0:
                EXPECT_TRUE(item.first)
                                    << "A node with key 0 should only be present once in the list, but false was returned.";
                EXPECT_FALSE(hasZero) << "A node with key 0 has already been captured.";
                hasZero = true;
                continue;
            case 1:
                if (item.first) {
                    EXPECT_FALSE(hasOne)
                                        << "A node with key 1 has been returned as a unique node, but it has already been captured before.";
                    EXPECT_FALSE(hasOnePtr);
                    hasOne = true;
                    onePtr = item.second;
                } else {
                    EXPECT_TRUE(
                            hasOne) << "A node with key 1 has not been captured yet, but it's been returned as a duplicate.";
                    EXPECT_FALSE(hasOnePtr);
                    EXPECT_EQ(onePtr, item.second)
                                        << "A pointer to wrong node has been returned for a node that has already been inserted.";
                    hasOnePtr = true;
                }
                continue;
            case 2:
                EXPECT_TRUE(item.first)
                                    << "A node with key 2 should only be present once in the list, but false was returned.";
                EXPECT_FALSE(hasTwo) << "A node with key 0 has already been captured.";
                hasTwo = true;
                continue;
            default:
                FAIL() << "An unexpected item has been added: " << item.second->key;
        }
    }

    ASSERT_TRUE(hasZero);
    ASSERT_TRUE(hasOne);
    ASSERT_TRUE(hasOnePtr);
    ASSERT_TRUE(hasTwo);
}

TEST_F(EmptyTree, LeavesCount) {
    std::vector<Node_t *> nodes;
    tree.GetLeafNodes(nodes);
    EXPECT_TRUE(nodes.empty()) << "An empty tree should have no leaves.";
}

TEST_F(EmptyTree, NonLeavesCount) {
    std::vector<Node_t *> nodes;
    tree.GetLeafNodes(nodes);
    EXPECT_TRUE(nodes.empty()) << "An empty tree should have no non-leaf nodes.";
}

TEST_F(EmptyTree, NodesCount) {
    std::vector<Node_t *> nodes;
    tree.GetAllNodes(nodes);
    EXPECT_TRUE(nodes.empty()) << "An empty tree should have no nodes.";
}

TEST_F(NonEmptyTree, InsertNode) {
    for (int val : uniqueValues) {
        std::pair<bool, Node_t *> item = tree.InsertNode(val);
        EXPECT_FALSE(item.first) << "A node with key " << val
                                 << " should've been already present, but adding it yielded true.";
        ASSERT_NE(item.second, nullptr) << "InsertNode returned a null pointer for key " << val;
        EXPECT_EQ(item.second->key, val) << "The key " << item.second->key <<
                                         " of the node returned by InsertNode doesn't match the queried value " << val;
    }

    std::pair<bool, Node_t *> item = tree.InsertNode(safeValue);

    EXPECT_TRUE(item.first) << "A node with key " << safeValue << " should not have been present.";
    ASSERT_NE(item.second, nullptr) << "InsertNode returned a null pointer for key " << safeValue;
    EXPECT_EQ(item.second->key, safeValue) << "The key " << item.second->key <<
                                           " of the node returned by InsertNode doesn't match the queried value "
                                           << safeValue;
}

TEST_F(NonEmptyTree, DeleteNode) {
    bool delResult = tree.DeleteNode(safeValue);
    EXPECT_FALSE(delResult) << "Delete returned true for " << safeValue << " which is not present in the tree.";

    for (int val : uniqueValues) {
        delResult = tree.DeleteNode(val);
        EXPECT_TRUE(delResult) << "Delete returned false for " << val << " which was present in the tree.";
    }

    for (int val : uniqueValues) {
        delResult = tree.DeleteNode(val);
        EXPECT_FALSE(delResult) << "Delete returned true for " << val
                                << " which was not supposed to be present in the tree.";
    }
}

TEST_F(NonEmptyTree, FindNode) {
    EXPECT_EQ(tree.FindNode(safeValue), nullptr) << "A non-existent node lookup should yield null.";

    std::pair<bool, BinaryTree::Node_t *> item = tree.InsertNode(safeValue);
    Node_t *lookup = tree.FindNode(safeValue);
    EXPECT_EQ(item.second, lookup) << "An item that had been added wasn't found.";

    for (int val : uniqueValues) {
        lookup = tree.FindNode(val);
        ASSERT_NE(lookup, nullptr) << "An item that had been added wasn't found.";
        EXPECT_EQ(lookup->key, val) << "The found item key doesn't match.";
    }
}

TEST_F(TreeAxioms, Axiom1) {
    std::vector<Node_t *> nodes;
    tree.GetAllNodes(nodes);

    ASSERT_GT(nodes.size(), 0) << "The tree has no nodes.";
    for (Node_t *node : nodes) {
        ASSERT_NE(node, nullptr) << "The tree has returned a null pointer for one of its nodes.";

        if (node->pLeft == nullptr && node->pRight == nullptr) {
            ASSERT_EQ(node->color, Color_t::BLACK) << "The colour of a node with no descendants is not black.";
        }
    }
}

TEST_F(TreeAxioms, Axiom2) {
    std::vector<Node_t *> nodes;
    tree.GetAllNodes(nodes);

    ASSERT_GT(nodes.size(), 0) << "The tree has no nodes.";
    for (Node_t *node : nodes) {
        ASSERT_NE(node, nullptr) << "The tree has returned a null pointer for one of its nodes.";

        if (node->color == Color_t::RED) {
            ASSERT_EQ(node->pLeft->color, Color_t::BLACK)
                                        << "The left descendant of the node with key " << node->key << " is not black.";
            ASSERT_EQ(node->pRight->color, Color_t::BLACK)
                                        << "The right descendant of the node with key " << node->key
                                        << " is not black.";
        }
    }
}

TEST_F(TreeAxioms, Axiom3) {
    std::vector<Node_t *> leafNodes;
    tree.GetLeafNodes(leafNodes);
    ASSERT_GT(leafNodes.size(), 0) << "The tree has no leaf nodes.";

    int matchingBlackNodesCount = -1;

    for (Node_t *leaf : leafNodes) {
        Node_t *root = tree.GetRoot();
        int blackNodesCount = 0;

        while (leaf != root) {
            if (leaf->color == Color_t::BLACK) {
                blackNodesCount++;
            }

            leaf = leaf->pParent;
        }

        if (matchingBlackNodesCount != -1) {
            EXPECT_EQ(blackNodesCount, matchingBlackNodesCount)
                                << "The number of black nodes doesn't match for node with key" << leaf->key;
        } else {
            matchingBlackNodesCount = blackNodesCount;
        }
    }
}

/*** Konec souboru black_box_tests.cpp ***/
