#ifndef NODE_H
#define NODE_H

#include "Token.h"

using namespace std;

class Node
{
    public:
        // Constructor for Node Class
        Node()
        {
            this->name = "UNKNOWN";
            this->parent = nullptr;
        }

        // Setters for name and parent members
        void setName(string newName)
        {
            this->name = newName; 
        }

        void setParent(Node* newParent)
        {
            this->parent = newParent;
        }

        // Getters for name, parent, and children vector members
        string getName()
        {
            return this->name;
        }

        Node* getParent()
        {
            return this->parent;
        }

        vector<Node*> getChildren()
        {
            return this->children;
        }

        // Returns a child at specific index for Tree generation
        Node* getChild(int index)
        {
            if (index < this->children.size())
            {
                return this->children[index];
            }
            else
            {
                return nullptr;
            }
        }

        // Returns whether or not the Node is linked to a token
        bool isTokenLinked()
        {
            return this->tokenPointer != nullptr;
        }

        // Sets the Node to a leaf node
        void setToLeaf()
        {
            this->isLeafNode = true;
        }

        // Returns whether or not node is a leaf node (used in AST)
        bool isLeaf()
        {
            return this->isLeafNode;
        }

        // Adds a child to the children vector
        void addChild(Node* childNode)
        {
            this->children.emplace_back(childNode);
        }

        // Adds the token pointer for this class
        void linkToken(Token* newToken)
        {
            this->tokenPointer = newToken;
        }

        // Returns the token pointer
        Token* getToken()
        {
            return this->tokenPointer;
        }

    private:
        // Members
        string name;
        Node* parent;
        vector<Node*> children;

        // Used to tell if node is a leaf node
        bool isLeafNode = false;

        // Optional member used for leaf nodes
        Token* tokenPointer = nullptr;
};

#endif