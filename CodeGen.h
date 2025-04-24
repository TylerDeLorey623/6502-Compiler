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
            // Traverse tree to generate code
            traverse(myAST->getRoot());

            // Create break at the end of the code
            write("00");

            // Backpatch temporary values
            backpatch();
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

        // Pointer for inserting hex in heap
        int heapVal = 0xff;

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

                    // Keeps track of if there were further traversals for this branch
                    bool furtherTraversals = false;

                    // Get the temporary location for the variable (T0, T1, etc)
                    string locationTemp = findVarIndex(locationValue->getName());

                    // Assigning statement is normal if second child is a leaf node
                    if (readValue->isLeaf())
                    {
                        // Write the value to accumulator
                        writeToRegister(readValue, "ACC");
                    }
                    // Do further traversing through the tree if there are further branches (ADD/isEq/isNotEq)
                    else
                    {
                        furtherTraversals = true;
                        traverse(readValue);
                    }

                    // Write calculated value (ID or literal) from accumulator into memory at locationTemp
                    write("8D");
                    write(locationTemp);
                    write("00");

                    // Reset temporary value used in further traversals
                    if (furtherTraversals)
                    {
                        furtherTraversals = false;
                        write("A9");
                        write("00");
                        write("8D");
                        write("FF");
                        write("00");
                    }
                }
                // Print Statement
                else if (name == "Print")
                {
                    // Get information about print
                    Node* printValue = node->getChild(0);
                    string type = getType(printValue);

                    // Keeps track of if there were further traversals for this branch
                    bool furtherTraversals = false;

                    // Print Statement is normal if child is just a leaf node
                    if (printValue->isLeaf())
                    {
                        // Writes value in print statement to the Y register
                        writeToRegister(printValue, "Y");
                    }
                    // If there are more branches, like addition or boolean expressions
                    else 
                    {
                        furtherTraversals = true;

                        // Traverse those
                        traverse(printValue);

                        // Write value into the Y register
                        write("AC");
                        write("FF");
                        write("00");
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

                    // Reset temporary value used in further traversals
                    if (furtherTraversals)
                    {
                        furtherTraversals = false;
                        write("A9");
                        write("00");
                        write("8D");
                        write("FF");
                        write("00");
                    }
                }
                // ADD branch
                else if (name == "ADD")
                {
                    // Get information about the addition
                    Node* firstValue = node->getChild(0);
                    Node* secondValue = node->getChild(1);

                    // If second value is another branch, traverse it first
                    if (!secondValue->isLeaf())
                    {
                        traverse(secondValue);
                    }
                    // If second value is a digit or ID
                    else
                    {
                        // Write value to memory location 0xFF (temporarily)
                        writeToRegister(secondValue, "ACC");
                        write("8D");
                        write("FF");
                        write("00");
                    }

                    // Write first value to accumulator
                    writeToRegister(firstValue, "ACC");
                    
                    // Perform add with temporary location 0xFF
                    write("6D");
                    write("FF");
                    write("00");

                    // Move value to temporary location 0xFF
                    write("8D");
                    write("FF");
                    write("00");
                }
            }
        }

        // Backpatches after code was generated
        void backpatch()
        {
            // Get real values into a vector
            vector<string> newValues;
            for (int i = 0, n = staticData.size(); i < n; i++)
            {
                log("DEBUG", "Backpatch: Variable '" + staticData[i].var + "' [T" + to_string(i) + "] with [" + toHex(pc) + "]");
                newValues.emplace_back(to_string(pc));
                pc++;
            }

            // Store index for runtime environment
            int index = 0;

            // Replace temporary values in runtime environment with real values
            for (string s : runEnv)
            {
                // Checks if its a temporary value
                if (s[0] == 'T')
                {
                    // Get real value (converts X in TX to an integer)
                    char val = s[1];
                    int correctIndex = val - '0';

                    // Replace temp value with real value
                    write(toHex(stoi(newValues[correctIndex])), index);
                }
                index++;
            }
        }

        // Write into the runtime environment at location of pc pointer (or specified index)
        void write(const string hex, int index = -1)
        {
            // If not writing into specified index, write it at program counter
            if (index == -1)
            {
                index = pc;

                // Increment program counter
                pc++;
            }
            
            // Insert hex (or temp value) to runtime environment code
            runEnv[index] = hex;
        }

        // Writes a value to a register
        void writeToRegister(Node* node, const string reg)
        {
            string constantCode = "00";
            string varCode = "00";

            // Find correct op codes depending on what register was specified
            // Accumulator
            if (reg == "ACC")
            {
                constantCode = "A9";
                varCode = "AD";
            }
            // X register
            else if (reg == "X")
            {
                constantCode = "A2";
                varCode = "AE";
            }
            // Y register
            else if (reg == "Y")
            {
                constantCode = "A0";
                varCode = "AC";
            }

            // Get variable/literal type
            string type = getType(node);

            // Check if its an ID
            if (node->getToken()->getType() == "ID")
            {
                // Load the register with the variable (or pointer)
                write(varCode);
                write(findVarIndex(node->getName()));
                write("00");
            }
            // If it wasn't an ID, it's a literal
            else
            {
                // Name of literal
                string literalName = node->getName();

                // Load register with constant
                write(constantCode);

                // If it is a string literal, create the string in heap
                if (type == "string")
                {
                    createString(node->getName());
                    write(toHex(heapVal + 1));
                }
                // If it is statically allocated
                else
                {
                    // Adds the literal to the runtime environment
                    addStaticLiteral(literalName);
                }
            }
        }

        // Writes a string into the heap in the runtime environment
        void createString(const string str)
        {
            // Update the heap pointer
            heapVal = heapVal - str.length() - 1;

            // Create temporary pointer
            int ptr = heapVal + 1;

            // For each character in the string
            for (char c : str)
            {
                // Convert to ASCII
                int asciiVal = c;

                // Write into heap
                write(toHex(asciiVal), ptr);

                ptr++;
            }
        }

        // Convert decimal to hexadecimal
        string toHex(int num)
        {
            // Convert to hex string (with correct padding)
            stringstream curChar;
            curChar << uppercase << hex << setw(2) << setfill('0') << num;
            return curChar.str();
        }

        // Gets the index of static data where variable is used
        string findVarIndex(const string varName)
        {
            int index = 0;
            int correctIndex = -1;

            // Find variable in staticData array
            for (VarNScope val : staticData)
            {
                // Check if variable name is correct
                if (val.var == varName)
                {
                    // Check if its in a reachable Scope
                    HashNode* node = currentHash;
                    string valScope = val.scope; 

                    while (node && node->getName() != valScope)
                    {
                        node = node->getParent();
                    }

                    // If it exists, set as candidate
                    if (node)
                    {
                        correctIndex = index;
                    }
                }
                index++;
            }

            // Returns last candidate (deepest Scope)
            return 'T' + to_string(correctIndex);
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
                cout << "Code Gen - ";

                cout << message << endl;
            }
        }
};

#endif