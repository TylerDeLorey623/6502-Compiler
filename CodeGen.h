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
            traverse(myAST->getRoot(), 0);

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
        struct ValNScope
        {
            string val;
            string scope;
            string type;

            // Constructor for struct
            ValNScope(const string v, const string s, const string t)
            {
                val = v;
                scope = s;
                type = t;
            }
        };

        // Vector to help with backpatching
        vector<ValNScope> staticData;

        // Keeps track of static data
        int lastStaticIndex = 0;

        // Pointer for inserting code in runtime environment
        int pc = 0x00;

        // Pointer for inserting hex in heap
        int heapVal = 0xff;

        // Traverses AST and generates hexadecimal code in runtime environment
        void traverse(Node* node, int depth)
        {
            // Get name of current Node (branch/leaf)
            string name = node->getName();
            string scopeName = currentHash->getName();

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
                    traverse(curNode, depth + 1);
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
                staticData.emplace_back(newVar, newScope, "VAR");
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
                bool furtherBOOL = false;
                string originalFE = "00";

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
                    traverse(readValue, depth + 1);
                }

                // Write calculated value (ID or literal) from accumulator into memory at locationTemp
                write("8D");
                write(locationTemp);
                write("00");
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
                    // Writes value in print statement to the Y register
                    writeToRegister(printValue, "Y");
                }
                // If there are more branches, like addition or boolean expressions
                else 
                {
                    traverse(printValue, depth + 1);

                    // Write traversed value into Y register using temporary address 0xFF
                    write("8D");
                    write("FF");
                    write("00");

                    // Write to Y register
                    write("AC");
                    write("FF");
                    write("00");

                    // Restore 0x00 at 0xFF
                    write("A9");
                    write("00");
                    write("8D");
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
            }
            // IF or WHILE branch
            else if (name == "If" || name == "While")
            {
                // Process boolean expression (isEq or isNotEq)
                traverse(node->getChild(0), depth + 1);
                
                // Compare accumulator value to 1
                // Write accumulator to 0xFF temporarily
                write("8D");
                write("FF");
                write("00");

                // Load 1 to X reg
                write("A2");
                write("01");

                // Compare values
                write("EC");
                write("FF");
                write("00");

                // Reset 0xFF to 0
                write("A9");
                write("00");
                write("8D");
                write("FF");
                write("00");

                // Branch n bytes over block if not equal (JP is placeholder for jump)
                write("D0");
                write("JP");

                // Get starting position
                int startPos = this->pc; 

                // Traverse the Block branch
                traverse(node->getChild(1), depth + 1);

                // Get ending position
                int endingPos = this->pc;

                // Find how long the block was
                int jumpAmount = endingPos - startPos;

                // Update jump placeholder
                write(toHex(jumpAmount), startPos - 1);
            }
            // ADD branch
            else if (name == "ADD")
            {
                // Add a temporary value to the end of the Stack that holds the sum 
                staticData.emplace_back("0", "0", name);
                lastStaticIndex = staticData.size() - 1;
                string currentTempAddress = "T" + to_string(lastStaticIndex);

                // Get information about the addition
                Node* firstValue = node->getChild(0);
                Node* secondValue = node->getChild(1);

                // If second value is another branch, traverse it first
                if (!secondValue->isLeaf())
                {
                    traverse(secondValue, depth + 1);
                }
                // If second value is a digit or ID
                else
                {
                    // Write value to temp memory location
                    writeToRegister(secondValue, "ACC");
                    write("8D");
                    write(currentTempAddress);
                    write("00");
                }

                // Write first value to accumulator
                writeToRegister(firstValue, "ACC");
                
                // Perform add with temporary location
                write("6D");
                write(currentTempAddress);
                write("00");

                // Move value to temporary location
                write("8D");
                write(currentTempAddress);
                write("00");
            }
            // isEq branch
            else if (name == "isEq" || name == "isNotEq")
            {
                // Adds two temporary values to the end of the Stack that holds boolean values (0 or 1)
                staticData.emplace_back("0", "0", name);
                lastStaticIndex = staticData.size() - 1;
                string tempAddress1 = "T" + to_string(lastStaticIndex);

                staticData.emplace_back("0", "0", name);
                lastStaticIndex = staticData.size() - 1;
                string tempAddress2 = "T" + to_string(lastStaticIndex);

                // Get information about two values being compared
                Node* firstValue = node->getChild(0);
                Node* secondValue = node->getChild(1);

                // If first value is another branch, traverse it first
                if (!firstValue->isLeaf())
                {
                    traverse(firstValue, depth + 1);
                }
                // If first value is an actual value, write to accumulator
                else
                {
                    writeToRegister(firstValue, "ACC");
                }

                // Write result of first value to temporary location 1
                write("8D");
                write(tempAddress1);
                write("00");

                // If second value is another branch, traverse it first
                if (!secondValue->isLeaf())
                {
                    traverse(secondValue, depth + 1);
                }
                // If second value is an actual value
                else
                {
                    // Write value into accumulator
                    writeToRegister(secondValue, "ACC");
                }

                // Write value into X register
                write("8D");
                write(tempAddress2);
                write("00");
                write("AE");
                write(tempAddress2);
                write("00");

                // Compare value in first temporary location to X register
                write("EC");
                write(tempAddress1);
                write("00");

                // Write a 0 into the accumulator if op was isEq
                write("A9");
                if (name == "isEq")
                {
                    write("00");   
                }
                // Write a 1 into the accumulator if op was isNotEq
                else 
                {
                    write("01");
                }

                // Branch 2 bytes if unequal
                write("D0");
                write("02");

                // If boolean expression was equal, set accumulator to 1 if op was isEq
                write("A9");
                if (name == "isEq")
                {
                    write("01");
                }
                else
                {
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
                string curType = staticData[i].type;
                if (curType == "VAR")
                {
                    log("DEBUG", "Backpatch: Variable '" + staticData[i].val + "' [T" + to_string(i) + "] with [" + toHex(pc) + "]");
                }
                else if (curType == "ADD")
                {
                    log("DEBUG", "Backpatch: Addition at [T" + to_string(i) + "] with [" + toHex(pc) + "]");
                }
                else if (curType == "If" || curType == "While" || curType == "isEq" || curType == "isNotEq")
                {
                    log("DEBUG", "Backpatch: Boolean at [T" + to_string(i) + "] with [" + toHex(pc) + "]");
                }
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
            for (ValNScope curVal : staticData)
            {
                // Check if variable name is correct
                if (curVal.type == "VAR" && curVal.val == varName)
                {
                    // Check if its in a reachable Scope
                    HashNode* node = currentHash;
                    string valScope = curVal.scope; 

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
            else if (tokenType == "CHAR" || tokenType == "QUOTE")
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