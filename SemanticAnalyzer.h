#ifndef ANALYZER_H
#define ANALYZER_H

using namespace std;

// The Semantic Analysis Class
class SemanticAnalyzer
{
    public:
        // Default constructor for the Analysis class
        SemanticAnalyzer(const int progNum, Tree* progCST)
        {
            this->programNumber = progNum;
            this->programCST = progCST;
            
            // Pointer for the AST
            myAST = new Tree();

            // Pointer for the Symbol Table
            mySym = new SymbolTable();
        }

        // Prints the AST
        void printAST()
        {
            if (VERBOSE)
            {
                log("INFO", "AST for Program #" + to_string(programNumber));
                // Initialize the result string
                traversalResult = "";

                // Make initial call to expand from root
                expand(myAST->getRoot(), 0);

                // Print results
                cout << traversalResult << endl;
            }
        }

        // Deletes AST from Memory so there are no memory leaks
        void deleteAST()
        {
            deleteNode(myAST->getRoot());
            delete(myAST);
            myAST = nullptr;
        }

        // Deletes Symbol Table from Memory so there are no memory leaks
        void deleteSymbolTable()
        {
            deleteHashNode(mySym->getRoot());
            delete(mySym);
            mySym = nullptr;
        }

        // Generates the AST based on CST by grabbing the important things from the CST
        // Also will generate the Symbol Table and do scope/type checking
        // There will be no syntax error checking since it is all already correct 
        void generate()
        {
            log("INFO", "Semantic Analysis for Program #" + to_string(programNumber));
            inorder(programCST->getRoot());
        }

        // Traverse Symbol Table to find more warnings
        void traverseSymbolTable()
        {
            traverseST(mySym->getRoot());
            log("INFO", "Semantic Analysis completed with " + to_string(errorCount) + " error(s) and " + to_string(warningCount) + " warning(s)");
        }

        // Print the Symbol Table in a neat and organized manner
        void printSymbolTable()
        {
            if (VERBOSE)
            {
                log("INFO", "Symbol Table for Program #" + to_string(programNumber));
                
                log("INFO", "NAME        TYPE        isINIT?        isUSED?        SCOPE");

                // Make initial call to expand from root
                printST(mySym->getRoot());
            }
        }

        // Returns error count
        int getErrors()
        {
            return errorCount;
        }

    private:
        // Default members
        int programNumber;
        Tree* programCST;

        // AST Members
        Tree* myAST;
        string traversalResult;

        // Symbol Table Pointer
        SymbolTable* mySym;

        // Tracks Scope
        int currentScope = -1;
        vector<string> subVals;

        int errorCount = 0;
        int warningCount = 0;

        // Deletes AST from Memory by deleting its Nodes recursively
        void deleteNode(Node* curNode)
        {
            if (curNode != nullptr)
            {
                vector<Node*> nodeChildren = curNode->getChildren(); 
                for (int i = 0, n = nodeChildren.size(); i < n; i++)
                {
                    deleteNode(nodeChildren[i]);
                }
                delete(curNode);
            }
        }

        // Deletes Symbol Table from Memory by deleting its HashNodes recursively
        void deleteHashNode(HashNode* curNode)
        {
            if (curNode != nullptr)
            {
                vector<HashNode*> nodeChildren = curNode->getChildren(); 
                for (int i = 0, n = nodeChildren.size(); i < n; i++)
                {
                    deleteHashNode(nodeChildren[i]);
                }
                delete(curNode);
            }
        }

        // In-order traversal of the CST to create the AST
        void inorder(Node* node)
        {
            // Makes sure node is not a nullptr
            if (!node)
            {
                return;
            }

            // Get the name of the Node (will initially be nullptr)
            string name = node->getName();

            // Checks if current Node is a branch
            if (!node->isTokenLinked())
            {
                // Entire IF statement does some logic depending on name of branch, usually involving recursively calling its child Nodes
                if (name == "Program")
                {
                    // Block
                    inorder(node->getChild(0));
                }
                else if (name == "Block")
                {
                    // Create new scope
                    currentScope++;
                    mySym->addHashNode(to_string(currentScope) + getScopeSubValue(currentScope));
                    
                    // Add node to AST
                    myAST->addNode("branch", "Block");
                    inorder(node->getChild(1));

                    // Move up the symbol table (and adjust scope)
                    mySym->moveUp();
                    currentScope--;

                    // Move up AST
                    myAST->moveUp();
                }
                else if (name == "Statement List")
                {
                    // If it wasn't an epsilon production
                    if (node->getChildren().size() != 0)
                    {
                        // Statement
                        inorder(node->getChild(0));

                        // Statement List
                        inorder(node->getChild(1));
                    }
                }
                else if (name == "Statement")
                {
                    // Type of Statement
                    inorder(node->getChild(0));
                }
                else if (name == "Print Statement")
                {
                    // Add print Node
                    myAST->addNode("branch", "Print");

                    // Expr
                    inorder(node->getChild(2));
                    
                    // Scope/type checking for print statements
                    checkPrint();

                    // Move up AST
                    myAST->moveUp();
                }
                else if (name == "Assignment Statement")
                {
                    // Add assignment Node
                    myAST->addNode("branch", "Assign");

                    // ID
                    inorder(node->getChild(0));

                    // EXPR
                    inorder(node->getChild(2));

                    // Scope/type checking for print statements
                    checkAssignment();

                    // Move up AST
                    myAST->moveUp();
                }
                else if (name == "Var Decl")
                {
                    // Add var decl Node
                    myAST->addNode("branch", "Declare");

                    // Type
                    inorder(node->getChild(0));

                    // ID
                    inorder(node->getChild(1));

                    // Scope/type checking for variable declaration statement
                    checkVarDecl();

                    // Move up AST
                    myAST->moveUp();
                }
                // Same logic for if/while statements
                else if (name == "If Statement" || name == "While Statement")
                {
                    // Add if/while Node (branch name is just If or While)
                    myAST->addNode("branch", name.erase(name.length() - 10, 10));

                    // Boolean Expr
                    inorder(node->getChild(1));

                    // Block
                    inorder(node->getChild(2));

                    // There is no semantic scope/type checking done for the IF/WHILE statements themselves
                    // but there will be for the boolean expr that they contain

                    // Move up AST
                    myAST->moveUp();
                }
                else if (name == "Expr")
                {
                    // Type of Expr
                    inorder(node->getChild(0));
                }
                else if (name == "Int Expr")
                {
                    // Determines whether IntExpr is...
                    // digit intop Expr
                    // or
                    // digit
                    if (node->getChildren().size() == 3)
                    {
                        // ADD branch
                        myAST->addNode("branch", "ADD");

                        // digit
                        inorder(node->getChild(0));

                        // Expr
                        inorder(node->getChild(2));

                        // Scope/type checking for ADD statements
                        checkADD();

                        // Move up AST
                        myAST->moveUp();
                    }
                    else
                    {
                        // digit
                        inorder(node->getChild(0));
                    }
                }
                // String Expr is a bit different
                // Collect all chars from the CharList to create the entire string
                else if (name == "String Expr")
                {
                    // Gets the entire string from the child CharLists
                    string result = "";
                    Token* currentToken = nullptr;
                    collectCharNodes(node->getChild(1), currentToken, result);
                    
                    // Add the string as a leaf node
                    myAST->addNode("leaf", result);
                    myAST->getMostRecentNode()->linkToken(currentToken);
                }
                else if (name == "Boolean Expr")
                {
                    // Determines whether BooleanExpr is...
                    // (Expr boolop Expr)
                    // or
                    // boolval
                    if (node->getChildren().size() == 5)
                    {
                        // Add a node based on equality sign (== or !=)
                        if (node->getChild(2)->getName() == "==")
                        {
                            myAST->addNode("branch", "isEq");
                        }
                        else
                        {
                            myAST->addNode("branch", "isNotEq");
                        }

                        // Expr
                        inorder(node->getChild(1));

                        // Expr
                        inorder(node->getChild(3));

                        // Scope/type checking for boolean expressions
                        checkBool();

                        // Move up AST
                        myAST->moveUp();
                    }
                    else
                    {
                        // boolval
                        inorder(node->getChild(0));
                    }
                }
                else if (name == "Id")
                {
                    // char
                    inorder(node->getChild(0));
                }
            }
            // If the current Node is a leaf node, add it to AST and link the correct Token
            else
            {
                Token* currentToken = node->getToken();
                myAST->addNode("leaf", node->getName());
                myAST->getMostRecentNode()->linkToken(currentToken);
            }
        }

        // Collect characters and format them into a single string
        // Passing result and token by reference
        void collectCharNodes(Node* node, Token*& token, string& result)
        {
            // Make sure it wasn't an epsilon production
            if (!node)
            {
                return;
            }

            // Gets the character or space
            string character = node->getChild(0)->getName();

            // Link token if it hasn't been linked yet (will occur when first character is added to result)
            if (!token)
            {
                token = node->getChild(0)->getToken();
            }

            // Add character or space to the result
            result += character;

            // Keep collecting CharLists
            collectCharNodes(node->getChild(1), token, result);
        }

        // Returns the type of a certain node
        string getType(Node* node)
        {
            string type = "UNKNOWN";
            string name = node->getName();

            // If node is a leaf, look for keywords
            if (node->isTokenLinked())
            {
                string tokenType = node->getToken()->getType();
                Token* linkedToken = node->getToken();
                
                // If it is an identifier, look up symbol table
                if (tokenType == "ID")
                {
                    // Get information about identifier
                    HashNode* curHashNode = mySym->getCurrentHashNode();
                    
                    // Find this identifier in the symbol table and get its type
                    // If the identifier wasn't found, error would be thrown in the corresponding check function
                    HashNode* correctNode = findInSymbolTable(curHashNode, node->getName());
                    if (correctNode)
                    {
                        type = correctNode->getType(name);
                    }
                }
                // Check if its a string literal
                else if (tokenType == "CHAR")
                {
                    type = "string";
                } 
                // Check if its an integer literal
                else if (tokenType == "DIGIT")
                {
                    type = "int";
                }
                // Check if its a boolean literal
                else if (tokenType == "BOOL_VAL")
                {
                    type = "boolean";
                }
            }
            // If node is a branch, check branch types
            else
            {
                // Check if it's an integer expression
                if (name == "ADD")
                {
                    type = "int";
                }
                // Check if it's a boolean expression 
                else if (name == "isEq" || name == "isNotEq")
                {
                    type = "boolean";
                }
            }

            return type;
        }

        // SCOPE/TYPE CHECKING FOR PRINT STATEMENTS
        void checkPrint()
        {
            // Get information about the current HashNode
            HashNode* curHashNode = mySym->getCurrentHashNode();

            // Get information about the current branch of AST for scope/type checking
            Node* currentBranch = myAST->getCurrentBranch();
            Node* child = currentBranch->getChild(0);
            string childName = child->getName();
            Token* linkedToken = child->getToken();

            // Checks to see if printing a variable 
            if (linkedToken->getType() == "ID")
            {
                // Find this variable in the symbol table
                HashNode* correctNode = findInSymbolTable(curHashNode, childName);

                // If variable exists, set it to used
                if (correctNode)
                {
                    correctNode->setUsed(childName);
                }
                // If it was not found, throw 'use of undeclared variable' error
                else
                {
                    log("ERROR", "Use of undeclared variable '" + childName + "' at (" + to_string(linkedToken->getLine()) + ":" + to_string(linkedToken->getColumn()) + ")");
                    errorCount++;
                }
            }
        }

        // SCOPE/TYPE CHECKING FOR ASSIGNMENT STATEMENTS
        void checkAssignment()
        {
            // Get information about the current HashNode and branch
            HashNode* curHashNode = mySym->getCurrentHashNode();
            Node* currentBranch = myAST->getCurrentBranch();

            // Get information about the variable that is getting assigned
            Node* targetNode = currentBranch->getChild(0);
            string targetName = targetNode->getName();
            Token* targetToken = targetNode->getToken();

            // Get information about the value the target is getting assigned to
            Node* valueNode = currentBranch->getChild(1);
            string valueName = valueNode->getName();
            Token* valueToken = nullptr;

            // Check to see if value is a leaf node (being assigned directly to either a variable or literal)
            if (valueNode->isTokenLinked())
            {
                valueToken = valueNode->getToken();
            }

            // Find the target variable in the symbol table
            HashNode* correctNode = findInSymbolTable(curHashNode, targetName);

            // If variable exists
            if (correctNode)
            {
                // Set it to initialized and link token
                correctNode->setInitialized(targetName);
                correctNode->setLineAndColumn(targetName, targetToken->getLine(), targetToken->getColumn());

                // Get types of both AST Nodes
                string targetType = getType(targetNode);
                string valueType = getType(valueNode);

                // Throw type mismatch error if types don't match
                if (targetType != valueType)
                {
                    // Type mismatch error when dealing with first ID and second ID
                    if (targetToken->getType() == "ID" && valueToken && valueToken->getType() == "ID")
                    {
                        log("ERROR", "Type mismatch: Assigning " + valueType + " value [" + valueName + "] to " + targetType + " variable [" + targetName + "] at (" + to_string(targetToken->getLine()) + ":" + to_string(targetToken->getColumn()) + ")");
                    }
                    // Type mismatch error when dealing with first ID and second literal
                    else if (valueToken)
                    {
                        log("ERROR", "Type mismatch: Assigning " + valueType + " literal [" + valueName + "] to " + targetType + " variable [" + targetName + "] at (" + to_string(targetToken->getLine()) + ":" + to_string(targetToken->getColumn()) + ")");
                    }
                    // Type mismatch error when dealing with first ID and second branch
                    else
                    {
                        log("ERROR", "Type mismatch: Assigning " + valueType + " value to " + targetType + " variable [" + targetName + "] at (" + to_string(targetToken->getLine()) + ":" + to_string(targetToken->getColumn()) + ")");
                    }
                    errorCount++;
                }
            }
            // If it was not found, throw 'use of undeclared variable' error
            else
            {
                log("ERROR", "Use of undeclared variable '" + targetName + "' at (" + to_string(targetToken->getLine()) + ":" + to_string(targetToken->getColumn()) + ")");
                errorCount++;
            }
        }

        // SCOPE/TYPE CHECKING FOR VARIABLE DECLARATION STATEMENTS
        void checkVarDecl()
        {
            // Get information about the current HashNode and branch
            HashNode* curHashNode = mySym->getCurrentHashNode();
            Node* currentBranch = myAST->getCurrentBranch();

            // Get the variable type and its name
            string type = currentBranch->getChild(0)->getName();
            string name = currentBranch->getChild(1)->getName();
            Token* token = currentBranch->getChild(0)->getToken();

            // Add the hash value at the current HashNode
            bool successful = curHashNode->addValue(name, type);

            // If there was a collision, throw error
            if (!successful)
            {
                log("ERROR", "Redeclared variable [" + name + "] at (" + to_string(token->getLine()) + ":" + to_string(token->getColumn()) + ")");
                errorCount++;
            }
            // Link tokens otherwise if creation was successful
            else
            {
                curHashNode->setLineAndColumn(name, token->getLine(), token->getColumn());
            }
        }

        // SCOPE/TYPE CHECKING FOR ADD STATEMENTS
        void checkADD()
        {
            // Get information about the current HashNode and branch
            HashNode* curHashNode = mySym->getCurrentHashNode();
            Node* currentBranch = myAST->getCurrentBranch();

            // Get information about the first "number" in addition
            Node* firstNode = currentBranch->getChild(0);
            string firstName = firstNode->getName();
            Token* firstToken = firstNode->getToken();

            // Get information about the second "number" in addition
            Node* secondNode = currentBranch->getChild(1);
            string secondName = secondNode->getName();
            Token* secondToken = nullptr;

            bool successful = false;

            // Check to see if second value is a leaf node (it would be another ADD branch otherwise for nested additions)
            if (secondNode->isTokenLinked())
            {
                secondToken = secondNode->getToken();

                // Check if the second "number" is an identifier
                if (secondToken->getType() == "ID")
                {
                    HashNode* correctNode = findInSymbolTable(curHashNode, secondName);
                    successful = correctNode;

                    // If it was successful, set to used
                    if (correctNode)
                    {
                        // Set it to used
                        correctNode->setUsed(secondName);
                    }
                }
                // If not an identifier, it's a literal
                else
                {
                    successful = true;
                }
            }
            // If it's a branch, successful since the branch has already been analyzed
            else
            {
                successful = true;
            }

            // If variable was found OR it branches again OR it's a literal, do type checking 
            if (successful)
            {
                // Get types of both AST Nodes
                string firstType = getType(firstNode);
                string secondType = getType(secondNode);

                // Throw type mismatch error if types aren't integers
                if (firstType != "int")
                {
                    log("ERROR", "Type mismatch: Using " + firstType + " in int expression at (" + to_string(firstToken->getLine()) + ":" + to_string(firstToken->getColumn()) + ")");
                    errorCount++;
                }
                else if (secondType != "int")
                {
                    log("ERROR", "Type mismatch: Using " + secondType + " in int expression at (" + to_string(secondToken->getLine()) + ":" + to_string(secondToken->getColumn()) + ")");
                    errorCount++;
                }
            }
            // If it was not found, throw 'use of undeclared variable' error
            else
            {
                log("ERROR", "Use of undeclared variable '" + secondName + "' at (" + to_string(secondToken->getLine()) + ":" + to_string(secondToken->getColumn()) + ")");
                errorCount++;
            }
        }

        // SCOPE/TYPE CHECKING FOR BOOLEAN EXPRESSIONS
        void checkBool()
        {
            // Get information about the current HashNode and branch
            HashNode* curHashNode = mySym->getCurrentHashNode();
            Node* currentBranch = myAST->getCurrentBranch();

            // Get information about the first expression
            Node* firstNode = currentBranch->getChild(0);
            string firstName = firstNode->getName();
            Token* firstToken = nullptr;

            bool firstSuccessful = false;

            // Check to see if first value is a leaf node (it would be another boolean expression branch otherwise for nested expressions)
            if (firstNode->isTokenLinked())
            {
                firstToken = firstNode->getToken();

                // Check if the second "number" is an identifier
                if (firstToken->getType() == "ID")
                {
                    HashNode* correctNode = findInSymbolTable(curHashNode, firstName);
                    firstSuccessful = correctNode;

                    // If it was successful, set to used
                    if (correctNode)
                    {
                        // Set it to used
                        correctNode->setUsed(firstName);
                    }
                }
                // If not an identifier, it's a literal
                else
                {
                    firstSuccessful = true;
                }
            }
            // If it's a branch, successful since the branch has already been analyzed
            else
            {
                firstSuccessful = true;
            }

            // Get information about the second expression
            Node* secondNode = currentBranch->getChild(1);
            string secondName = secondNode->getName();
            Token* secondToken = nullptr;

            bool secondSuccessful = false;

            // Check to see if second value is a leaf node (it would be another boolean expression branch otherwise for nested expressions)
            if (secondNode->isTokenLinked())
            {
                secondToken = secondNode->getToken();

                // Check if the second "number" is an identifier
                if (secondToken->getType() == "ID")
                {
                    HashNode* correctNode = findInSymbolTable(curHashNode, secondName);
                    secondSuccessful = correctNode;

                    // If it was successful, set to used
                    if (correctNode)
                    {
                        // Set it to used
                        correctNode->setUsed(secondName);
                    }
                }
                // If not an identifier, it's a literal
                else
                {
                    secondSuccessful = true;
                }
            }
            // If it's a branch, successful since the branch has already been analyzed
            else
            {
                secondSuccessful = true;
            }

            // If variables were found OR they branch again OR they are literals, do type checking 
            if (firstSuccessful && secondSuccessful)
            {
                // Get types of both AST Nodes
                string firstType = getType(firstNode);
                string secondType = getType(secondNode);

                // Throw type mismatch error if types aren't integers
                if (firstType != "boolean")
                {
                    log("ERROR", "Type mismatch: Using " + firstType + " in boolean expression at (" + to_string(firstToken->getLine()) + ":" + to_string(firstToken->getColumn()) + ")");
                    errorCount++;
                }
                else if (secondType != "boolean")
                {
                    log("ERROR", "Type mismatch: Using " + secondType + " in boolean expression at (" + to_string(secondToken->getLine()) + ":" + to_string(secondToken->getColumn()) + ")");
                    errorCount++;
                }
            }
            // If it was not found, throw 'use of undeclared variable' error
            else
            {
                log("ERROR", "Use of undeclared variable '" + secondName + "' at (" + to_string(secondToken->getLine()) + ":" + to_string(secondToken->getColumn()) + ")");
                errorCount++;
            }
        }

        // Finds a variable in symbol table starting at hashnode
        // Returns false if nullptr, returns true if its valid
        HashNode* findInSymbolTable(HashNode* node, string varName)
        {
            bool inTable = false;

            // Finds the variable (if it exists)
            while (node != nullptr)
            {
                inTable = node->exists(varName);
                if (inTable)
                {
                    break;
                }
                node = node->getParent();
            }

            return node;
        }

        // Gets the subvalue of a scope (Scope 1 may have 1a, 1b, etc)
        string getScopeSubValue(int scopeVal)
        {
            // If this is a new scope, add a new string to the scope vector
            if (subVals.size() == scopeVal)
            {
                subVals.emplace_back("");
            }

            // If this is an existing scope (Besides scope 0, as there will always be only one scope 0)
            if (scopeVal != 0)
            {
                // Get previous letter
                string currentLetter = subVals[scopeVal];
                string newLetter;

                // If there was no previous letter, set it to a
                if (currentLetter == "")
                {
                    newLetter = "a";
                }
                // Otherwise, increment the scope letter
                else
                {
                    // Increment any letter if not a z
                    if (currentLetter.back() != 'z')
                    {
                        currentLetter.back() = currentLetter.back() + 1;
                        newLetter = currentLetter;
                    }
                    // If the letter is z, increment previous letter as well (loop)
                    else
                    {
                        // Says whether or not to increment selected letter based letter after it
                        bool increment = false;

                        // Loops through each letter from the back of the string to the front
                        for (int i = currentLetter.length() - 1; i >= 0; i--)
                        {
                            char selectedLetter = currentLetter[i];

                            // If the letter needs to be incremented and its not a z, increment it
                            if (increment && selectedLetter != 'z')
                            {
                                currentLetter[i] = currentLetter[i] + 1;
                                increment = false;
                            }

                            // If the letter is a 'z', set it to 'a' and add signal to increment previous letter as well
                            if (selectedLetter == 'z')
                            {
                                currentLetter[i] = 'a';
                                increment = true;
                            }
                        }

                        // Insert an 'a' at the beginning of string if first letter was a 'z'
                        if (increment)
                        {
                            increment = false;
                            currentLetter.insert(currentLetter.begin(), 'a');
                        }

                        newLetter = currentLetter;
                    }
                }

                subVals[scopeVal] = newLetter;
            }

            // Return the new scope letter
            return subVals[scopeVal]; 
        }

        // Function that traverses symbol table to check for the "unused" warnings
        void traverseST(HashNode* node)
        {
            // Check each hash at this scope
            warningCount += node->warningCheck();

            // Recursively expand the branches
            for (int i = 0, childrenSize = node->getChildren().size(); i < childrenSize; i++)
            {
                traverseST(node->getChild(i));
            }
        }

        // Function that traverses ST and prints it out
        void printST(HashNode* node)
        {
            bool currentHashComplete = false;
            int index = 0;

            node->printCurrentHash();

            // Recursively expand the branches
            for (int i = 0, childrenSize = node->getChildren().size(); i < childrenSize; i++)
            {
                printST(node->getChild(i));
            }
        }

        // Most of the expand() function references code by Alan G. Labouseur, based on the 2009 work by Michael Ardizzone and Tim Smith.
        // Recursive function to handle the expansion of the nodes
        void expand(Node* curNode, int depth)
        {
            // Beginning of each line
            traversalResult += "INFO    AST: ";

            // Space out based on the current depth so
            // this looks at least a little tree-like.
            for (int i = 0; i < depth; i++)
            {
                traversalResult += "-";
            }

            // Check if current node is a leaf node
            if (curNode->isLeaf())
            {
                // note the leaf node
                traversalResult += "[" + curNode->getName() + "] \n";
            }
            else
            {
                // If not, note the branch node, and expand on its children
                traversalResult += "<" + curNode->getName() + "> \n";

                // Recursively expand the branches
                for (int i = 0, childrenSize = curNode->getChildren().size(); i < childrenSize; i++)
                {
                    expand(curNode->getChild(i), depth + 1);
                }
            }
        }

        // Logging function for Analyzer
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