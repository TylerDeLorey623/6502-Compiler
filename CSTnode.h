#ifndef CSTNODE_H
#define CSTNODE_H

#include <iostream>
#include <vector>
#include "Token.h"

using namespace std;

class CSTnode
{
    public:
        // Constructor for CSTnode Class
        CSTnode()
        {
            this->name = "UNKNOWN";
            this->parent = nullptr;
        }

        // Setters for name and parent members
        void setName(string newName)
        {
            this->name = newName; 
        }

        void setParent(CSTnode* newParent)
        {
            this->parent = newParent;
        }

        // Getters for name, parent, and children vector members
        string getName()
        {
            return this->name;
        }

        CSTnode* getParent()
        {
            return this->parent;
        }

        vector<CSTnode*> getChildren()
        {
            return this->children;
        }

        // Returns a child at specific index for CST generation
        CSTnode* getChild(int index)
        {
            return this->children[index];
        }

        // Adds a child to the children vector
        void addChild(CSTnode* childNode)
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
        CSTnode* parent;
        vector<CSTnode*> children;

        // Optional member used for leaf nodes
        Token* tokenPointer = nullptr;
};

#endif