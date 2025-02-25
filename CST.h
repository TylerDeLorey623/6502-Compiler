#ifndef CST_H
#define CST_H

#include "CSTnode.h"

using namespace std;

class CST
{
    public:
        CST()
        {
            this->root = nullptr;
            this->current = nullptr;
        }

        // Adds a Node to the CST
        void addNode(string kind, string label)
        {
            // Creates a new Node and sets its name to the label
            CSTnode* newNode = new CSTnode();
            newNode->setName(label);

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

            // Sets the current Node of the CST to this new Node if it's not a leaf node
            if (kind != "leaf")
            {
                this->current = newNode;
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

    private:
        CSTnode* root;
        CSTnode* current;
};

#endif