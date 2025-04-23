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

        // Keeps track of static data
        int lastStaticIndex = 0;

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
                    lastStaticIndex = staticData.size() - 1;

                    // If the data type is either an int or a boolean, add code that initializes it to 0 (which is false)
                    if (newType != "string")
                    {
                        // Load the accumulator with 0
                        write("A9");
                        write("00");

                        // Store the accumulator in temporary memory location (little endian, will always begin with 00 since highest memory location is 0x00ff, which is 0xff)
                        write("8D");
                        write("T" + to_string(lastStaticIndex));
                        write("00");
                    }
                }
                // Assignment Statement
                else if (name == "Assign")
                {
                    // Get information about assignment
                    Node* locationValue = node->getChild(0);
                    Node* readValue = node->getChild(1);

                    int locationVal = findVarIndex(locationValue->getName());

                    // Assigning statement is normal if second child is a leaf node
                    if (readValue->isLeaf())
                    {
                        // Get variable type of assigned variable
                        string locationVarType = getType(locationValue);

                        // If the location variable uses static allocation
                        if (locationVarType != "string")
                        {
                            // Check if its an ID
                            if (readValue->getToken()->getType() == "ID")
                            {
                                // Load the readValue from memory
                                write("AD");
                                write("T" + to_string(findVarIndex(readValue->getName())));
                                write("00");
                            }
                            // If it wasn't an ID, it's a literal
                            else
                            {
                                // Name of literal
                                string literalName = readValue->getName();

                                // Load accumulator with constant
                                write("A9");

                                // Adds the literal to the runtime environment
                                addStaticLiteral(literalName);
                            }
                            // Write this value (ID or literal) into memory at locationVal
                            write("8D");
                            write("T" + to_string(locationVal));
                            write("00");
                        }
                    }
                }
                // Print Statement
                else if (name == "Print")
                {
                    // Get information about print
                    Node* printValue = node->getChild(0);
                    string type = getType(printValue);

                    // Print Statement is normal if child is just a leaf node
                    if (printValue->isLeaf())
                    {
                        // Check if printed value is an ID
                        if (printValue->getToken()->getType() == "ID")
                        {
                            // Load Y register with contents of variable
                            write("AC");
                            write("T" + to_string(findVarIndex(printValue->getName())));
                            write("00");
                        }
                        // If it's not an ID, it's a literal
                        else
                        {
                            // Load Y register with constant
                            write("A0");

                            // If it is not a string
                            if (printValue->getToken()->getType() != "CHAR")
                            {
                                // Adds the static literal to the runtime environment
                                addStaticLiteral(printValue->getName());
                            }
                        }

                        // Loads either a 1 or 2 into X register depending on static allocation
                        write("A2");

                        // Loads a 1 in the X register if it is not a string
                        if (type != "string")
                        {
                            write("01");
                        }
                        // Loads a 2 in the X register if it is a string
                        else
                        {
                            write("02");
                        }

                        // System call
                        write("FF");
                    }
                }
            }
        }

        // Write into the runtime environment at location of pc pointer
        void write(const string hex)
        {
            // Insert hex (or temp value) to runtime environment code
            runEnv[pc] = hex;

            // Increment program counter
            pc++;
        }

        // Gets the index of static data where variable is used
        int findVarIndex(const string varName)
        {
            int index = 0;
            int correctIndex = -1;

            // Find variable in staticData array
            for (VarNScope val : staticData)
            {
                if (val.var == varName)
                {
                    correctIndex = index;
                }
                index++;
            }

            return correctIndex;
        }

        // Gets the type of a variable from the symbol table
        string getType(Node* curNode)
        {
            string type = "UNKNOWN";
            string name = curNode->getName();
            string tokenType = curNode->getToken()->getType();

            // If it is an identifier, look up symbol table
            if (tokenType == "ID")
            {
                // Finds the variable
                HashNode* node = currentHash;
                while (!node->exists(curNode->getName()))
                {
                    node = node->getParent();
                }

                // Gets variable type
                type = node->getType(name);
            }
            // Check if its a string literal
            else if (tokenType == "CHAR")
            {
                type = "string";
            } 
            // Check if its an integer literal or ADD block
            else if (tokenType == "DIGIT" || name == "ADD")
            {
                type = "int";
            }
            // Check if its a boolean literal or boolean block
            else if (tokenType == "BOOL_VAL" || name == "isEq" || name == "isNotEq")
            {
                type = "boolean";
            }

            return type;
        }

        // Converts false/true to 0/1 respectively and writes it (and writes integers)
        void addStaticLiteral(string name)
        {
            // Convert false/true to 0/1 respectively
            if (name == "false")
            {
                write("00");
            }
            else if (name == "true")
            {
                write("01");
            }
            // Otherwise its a literal integer (0-9, so pad with 0)
            else 
            {
                write("0" + name);
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