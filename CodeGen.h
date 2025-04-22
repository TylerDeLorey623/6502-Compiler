#ifndef CODEGEN_H
#define CODEGEN_H

using namespace std;

// Code Generation Class
class CodeGen
{
    public:
        // Default constructor for the CodeGen class
        CodeGen(const int progNum, Tree* progAST, SymbolTable* progSymTable)
        {
            this->programNumber = progNum;
            this->myAST = progAST;
            this->mySymTable = progSymTable;

            this->currentHash = mySymTable->getRoot();
            
            // Fill runtime environment will all 0x00
            fill(runEnv.begin(), runEnv.end(), "00");
        }

        // Starts generating code
        void generate()
        {
            traverse(myAST->getRoot());
        }

        // Print runtime environment
        void print()
        {
            log("INFO", "Code Generation for Program #" + to_string(programNumber));

            // Divide output every eight lines
            for (int i = 0x00; i < 0x100; i++)
            {
                if (i != 0x00 && i % 0x08 == 0x00)
                {
                    cout << endl;
                }

                cout << runEnv[i] << " ";
            }
            cout << endl;
        }

    private:
        // RUNTIME ENVIRONMENT (indices range from 0 - 255)
        array<string, 0x100> runEnv;

        // Default members
        int programNumber;
        Tree* myAST;
        SymbolTable* mySymTable;

        HashNode* currentHash;

        // Struct that helps with static data storage
        struct VarNScope
        {
            string var;
            string scope;

            // Constructor for struct
            VarNScope(const string v, const string s)
            {
                var = v;
                scope = s;
            }
        };

        // Vectors to help with backpatching
        vector<VarNScope> staticData;
        vector<int> jumps;

        // Pointer for inserting code in runtime environment
        int pc = 0x00;

        // Traverses AST and generates hexadecimal code in runtime environment
        void traverse(Node* node)
        {
            // Get name of current Node (branch/leaf)
            string name = node->getName();
            string scopeName = currentHash->getName();

            // If the node is a branch
            if (!node->isLeaf())
            {
                // Recursively call this function for all children of a Block
                if (name == "Block")
                {
                    // Increase scope to the first non-traversed scope in its children 
                    if (node->getParent())
                    {
                        vector<HashNode*> hashChildren = currentHash->getChildren();
                        int index = 0;
                        do
                        {
                            currentHash = hashChildren[index];
                            index++;
                        } 
                        while (currentHash->checkTraversed());
                    }

                    // Traverse through the current AST branch's children
                    for(Node* curNode : node->getChildren())
                    {
                        traverse(curNode);
                    }

                    // Note that current Symbol Table was traversed
                    currentHash->setTraversed();

                    // Move up the Symbol Table Tree
                    currentHash = currentHash->getParent();
                }
                // Variable declaration
                else if (name == "Declare")
                {
                    // Add value in static data vector
                    string newType = node->getChild(0)->getName();
                    string newVar = node->getChild(1)->getName();
                    string newScope = currentHash->getName();
                    staticData.emplace_back(newVar, newScope);

                    
                }
            }
            // If the node is a leaf
            else
            {

            }
        }

        // Logging function for CodeGen
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
                cout << "Code Generation - ";

                cout << message << endl;
            }
        }
};

#endif 