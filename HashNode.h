#ifndef HASHNODE_H
#define HASHNODE_H

using namespace std;

class HashNode
{
    public:
        // Constructor for HashNode Class
        HashNode(string newName)
        {
            this->name = newName;
            this->parent = nullptr;
        }

        // Adds a new value to the hash table
        void addValue(string newVar, string newType)
        {
            this->hashTable[newVar] = {newType, false, false};
        }

        // Setter for parent node
        void setParent(HashNode* newParent)
        {
            this->parent = newParent;
        }

        // Adds a child to the children vector
        void addChild(HashNode* childNode)
        {
            this->children.emplace_back(childNode);
        }

        // Getter for HashNode name
        string getName()
        {
            return this->name;
        }

        // Getter for parent node
        HashNode* getParent()
        {
            return this->parent;
        }

        // Getter for children vector
        vector<HashNode*> getChildren()
        {
            return this->children;
        }

    private:
        // Creates a struct that stores object information for each value in hash table
        struct hashObject
        {
            string type;
            bool isInitialized;
            bool isUsed;
        };

        // unordered_map has hashing capabilities
        // Hash table (variable, (type, isInitialized, isUsed))
        unordered_map<string, hashObject> hashTable;

        // Other members
        string name;
        HashNode* parent;
        vector<HashNode*> children; 
};

#endif