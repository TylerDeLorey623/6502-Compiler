#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "HashNode.h"

using namespace std;

class SymbolTable
{
    public:
        // Default constructor for Symbol Table
        SymbolTable()
        {
            this->root = nullptr;
            this->current = nullptr;
        }

        // Adds a Hashmap to the Symbol Table
        void addHashNode(string name)
        {
            // Creates a new Node and sets its initial values
            HashNode* newNode = new HashNode(name);

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

            // Sets the current Node of the Tree to this new Node
            this->current = newNode;
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
        HashNode* getRoot()
        {
            return this->root;
        }

        // Getter for current Node
        HashNode* getCurrentHashNode()
        {
            return this->current;
        }

    private:
        HashNode* root;
        HashNode* current;
};

#endif