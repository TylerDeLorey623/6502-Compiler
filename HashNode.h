#ifndef HASHNODE_H
#define HASHNODE_H

using namespace std;

class HashNode
{
    public:
        // Parameterized Constructor for HashNode Class
        HashNode(string newVar, string newType)
        {
            hashTable[newVar] = {newType, false, false};
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
};

#endif