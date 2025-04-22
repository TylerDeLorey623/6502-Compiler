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

        // Adds a new value to the hash table and returns if it was successful (no collision)
        bool addValue(string newVar, string newType)
        {
            bool success = false;
            if (!exists(newVar))
            {
                this->hashTable[newVar] = {newType, false, false};
                success = true;
            }
            return success;
        }

        // Checks if values were used and initialized
        int warningCheck()
        {
            int warningCount = 0;

            // Loop through all hashes in this Node of the symbol table
            for (const auto& val : hashTable)
            {
                // Declared but not initialized
                if (!val.second.isInitialized)
                {
                    log("WARNING", val.second.type + " [" + val.first + "] is declared at (" + to_string(getLine(val.first)) + ":" + to_string(getColumn(val.first)) + "), but never initialized");
                    warningCount++;
                }
                // Initialized but not used
                else if (!val.second.isUsed)
                {
                    log("WARNING", val.second.type + " [" + val.first + "] is declared at (" + to_string(getLine(val.first)) + ":" + to_string(getColumn(val.first)) + ") and was also initialized, but never used");
                    warningCount++;
                }
            }
            return warningCount;
        }

        // Set line and column for Hash Value
        void setLineAndColumn(string name, int newLine, int newColumn)
        {
            this->hashTable[name].line = newLine;
            this->hashTable[name].column = newColumn;
        }

        // Setter for parent node
        void setParent(HashNode* newParent)
        {
            this->parent = newParent;
        }

        // Setter that this HashNode was used
        void setUsed(string name)
        {
            this->hashTable[name].isUsed = true;
        }

        // Setter that this HashNode is initialized
        void setInitialized(string name)
        {
            this->hashTable[name].isInitialized = true;
        }

        // Adds a child to the children vector
        void addChild(HashNode* childNode)
        {
            this->children.emplace_back(childNode);
        }

        // Getter for line number
        int getLine(string name)
        {
            return this->hashTable[name].line;
        }

        // Getter for column number
        int getColumn(string name)
        {
            return this->hashTable[name].column;
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

        // Getter for a specific child at index
        HashNode* getChild(int index)
        {
            return this->children[index];
        }

        // Getter for a certain type
        string getType(string name)
        {
            return this->hashTable[name].type;
        }

        // Gets whether or not variable was initialized
        bool checkInitialized(string name)
        {
            return this->hashTable[name].isInitialized;
        }

        // Gets whether or not variable was used
        bool checkUsed(string name)
        {
            return this->hashTable[name].isUsed;
        }

        // Returns whether or not a value in the hash table exists
        bool exists(string name)
        {
            bool ifExists = false;

            if (this->hashTable.find(name) != this->hashTable.end())
            {
                ifExists = true;
            }

            return ifExists;
        } 

        // Print out every value of all hash tables
        void printCurrentHash()
        {
            for (const auto& val : hashTable)
            {
                string name = val.first;
                string type = val.second.type;
                string initialized = "true";
                if (!val.second.isInitialized)
                {
                    initialized = "false";
                }
                string used = "true";
                if (!val.second.isUsed)
                {
                    used = "false";
                }
                string scope = getName();
                string line = to_string(getLine(name));
                string column = to_string(getColumn(name));

                // Format the string to look nice
                int spaces = 8;

                // NAME
                string completeString = name;
                for (int i = 0; i < spaces + 4 - name.size(); i++)
                {
                    completeString += " ";
                }
                // TYPE
                completeString += type;
                for (int i = 0; i < spaces + 4 - type.size(); i++)
                {
                    completeString += " ";
                }
                // ISINIT?
                completeString += initialized;
                for (int i = 0; i < spaces + 7 - initialized.size(); i++)
                {
                    completeString += " ";
                }
                // ISUSED?
                completeString += used;
                for (int i = 0; i < spaces + 7 - used.size(); i++)
                {
                    completeString += " ";
                }
                // SCOPE
                completeString += scope;
                for (int i = 0; i < spaces + 5 - scope.size(); i++)
                {
                    completeString += " ";
                }
                // LINE
                completeString += line;
                for (int i = 0; i < spaces + 4 - line.size(); i++)
                {
                    completeString += " ";
                }
                // COLUMN
                completeString += column;

                log("INFO", completeString);
            }
        }

        // Check if table was traversed for Code Gen
        bool checkTraversed()
        {
            return this->traversed;
        }

        // Set traversed when traversed in Code Gen
        void setTraversed()
        {
            this->traversed = true;
        }

    private:
        // Creates a struct that stores object information for each value in hash table
        struct hashObject
        {
            string type;
            bool isInitialized;
            bool isUsed;
            int line;
            int column;
        };

        // unordered_map has hashing capabilities
        // Hash table (variable, (type, isInitialized, isUsed))
        unordered_map<string, hashObject> hashTable;

        // Other members
        string name;
        HashNode* parent;
        vector<HashNode*> children; 

        // Member used for Symbol Table
        bool traversed = false;

        // Logging function for Analyzer used for Hash Table warnings
        void log(const string type, const string message)
        {
            // Only outputs if verbose mode is on or its INFO
            if (VERBOSE || type != "DEBUG")
            {
                // For good looking formatting
                const int spaceCount = 8;
                const int spaces = spaceCount - type.length();
                if (spaces <= 0)
                {
                    return;
                }
                
                // Print type
                cout << type;

                // Adds correct number of spaces so all the messages line up.
                for (int i = 0; i < spaces; i++)
                {
                    cout << " ";
                }
                cout << "Analyzer - ";

                cout << message << endl;
            }
        }
};

#endif