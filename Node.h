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
            return this->children[index];
        }

        // Returns whether or not the Node is linked to a token
        bool isTokenLinked()
        {
            return this->tokenPointer != nullptr;
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

    private:
        // Members
        string name;
        Node* parent;
        vector<Node*> children;

        // Optional member used for leaf nodes
        Token* tokenPointer = nullptr;
};

#endif