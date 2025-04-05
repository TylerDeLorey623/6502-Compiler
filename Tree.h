#ifndef TREE_H
#define TREE_H

#include "Node.h"

using namespace std;

class Tree
{
    public:
        // Default constructor for Tree (CST/AST)
        Tree()
        {
            this->root = nullptr;
            this->current = nullptr;
            this->mostRecentNode = nullptr;
        }

        // Adds a Node to the Tree
        void addNode(string kind, string label)
        {
            // Creates a new Node and sets its name to the label
            Node* newNode = new Node();
            newNode->setName(label);

            mostRecentNode = newNode;

            // If there is no root Node, this Node becomes the root (and its parent is null)
            if (this->root == nullptr)
            {
                this->root = newNode;
                newNode->setParent(nullptr);
            }
            // If this is not the root Node, make its parent the "current" member Node
            // and add the newNode as a child to the "current" Node
            else
            {
                newNode->setParent(current);
                current->addChild(newNode);
            }

            // Sets the current Node of the Tree to this new Node if it's not a leaf node
            if (kind != "leaf")
            {
                this->current = newNode;
            }
            // If it's a leaf node, note so
            else
            {
                newNode->setToLeaf();
            }
        }

        // Moves the current Node up the tree
        void moveUp()
        {
            if (this->current->getParent() != nullptr)
            {
                this->current = this->current->getParent();
            }
        }

        // Getter for the root Node
        Node* getRoot()
        {
            return this->root;
        }

        // Getter for most recent Node
        Node* getMostRecentNode()
        {
            return this->mostRecentNode;
        }

        // Getter for branch at the same level
        Node* getCurrentBranch()
        {
            return this->current;
        }

    private:
        Node* root;
        Node* current;

        // Pointer for the most recent Node (used for linking tokens to leaf nodes)
        Node* mostRecentNode;
};

#endif