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
                // Set it to initialized
                correctNode->setInitialized(targetName);

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
        }

        // Does scope/type checking on the current branch
        void semanticCheck()
        {
            // Gets current branch information and its children
            Node* curBranch = myAST->getCurrentBranch();
            string branchName = curBranch->getName();
            Node* leaf1;
            Node* leaf2;

            // Extra variables needed for scope/type checking
            HashNode* correctNode;
            Token* linkedToken1;
            Token* linkedToken2;
            bool successful = false;
            string name1;
            string name2;
            string type1 = "UNKNOWN";
            string type2 = "UNKNOWN";

            // Move up tree if scope was changed
            if (branchName == "Block")
            {
                mySym->moveUp();
                currentScope--;
            }

            // Get information about the current HashNode
            HashNode* curHashNode = mySym->getCurrentHashNode();

            // Scope/type checking for each type of statement
            if (branchName == "ADD")
            {
                // Find the overarching statement
                Node* overallBranch = curBranch;
                while (overallBranch->getName() == "ADD")
                {
                    overallBranch = overallBranch->getParent();
                } 
                
                // Does semantic checking for both leaves of the ADD
                checkWithADDs(curBranch->getChild(0), curHashNode, overallBranch);
                checkWithADDs(curBranch->getChild(1), curHashNode, overallBranch);
            }
            // Scope check to see if variable exists
            else if (branchName == "Print Statement")
            {
                // Set the variable to used if it exists
                leaf1 = curBranch->getChild(0);
                name1 = leaf1->getName();
                linkedToken1 = leaf1->getToken();

                // Don't do scope checking if its an ADD branch (since it has already been checked)
                if (name1 != "ADD" && linkedToken1->getType() == "ID")
                {
                    correctNode = findInSymbolTable(curHashNode, name1);
                    successful = correctNode;

                    // Sets the variable to used if it was found
                    if (successful)
                    {
                        correctNode->setLineAndColumn(name1, linkedToken1->getLine(), linkedToken1->getColumn());
                        correctNode->setUsed(name1);
                    }
                    else
                    {
                        log("ERROR", "Use of undeclared variable '" + name1 + "' at (" + to_string(linkedToken1->getLine()) + ":" + to_string(linkedToken1->getColumn()) + ")");
                        errorCount++;
                    }
                }
            }
            // Find symbol in symbol table and type check
            else if (branchName == "Assignment Statement" || branchName == "isEq" || branchName == "isNotEq")
            {
                bool noErrors = true;

                // Get information for both leaves
                leaf1 = curBranch->getChild(0);
                name1 = leaf1->getName();
                linkedToken1 = leaf1->getToken();

                if (curBranch->getChildren().size() >= 2)
                {
                    leaf2 = curBranch->getChild(1);
                    name2 = leaf2->getName();
                    linkedToken2 = leaf2->getToken();
                }
                // FIX THIS
                else
                {
                    leaf2 = curBranch->getChild(0);
                    name2 = leaf1->getName();
                    linkedToken2 = leaf1->getToken();
                }

                // Scope check the identifier
                if (name1 != "ADD" && linkedToken1->getType() == "ID")
                {
                    correctNode = findInSymbolTable(curHashNode, name1);
                    successful = correctNode;

                    // If variable is in symbol table, get its type and set it to initialized or used
                    if (successful)
                    {
                        type1 = correctNode->getType(name1);
                        correctNode->setLineAndColumn(name1, linkedToken1->getLine(), linkedToken1->getColumn());

                        // If it is a comparison, set it to used
                        if (branchName != "Assignment Statement")
                        {
                            correctNode->setUsed(name1);

                            // Throw warning if it wasn't initialized yet
                            if (!correctNode->checkInitialized(name1))
                            {
                                log("WARNING", "Using uninitialized variable " + name1 + " at (" + to_string(linkedToken1->getLine()) + ":" + to_string(linkedToken1->getColumn()) + ")");
                            }
                        }
                    }
                    // If variable wasn't in symbol table, throw undeclared variable error
                    else
                    {
                        log("ERROR", "Use of undeclared variable '" + name1 + "' at (" + to_string(linkedToken1->getLine()) + ":" + to_string(linkedToken1->getColumn()) + ")");
                        errorCount++;
                        noErrors = false;
                    }
                }

                // Also scope check the second leaf to see its an identifier, not just an int, string, or boolean
                if (name2 != "ADD" && linkedToken2->getType() == "ID")
                {
                    correctNode = findInSymbolTable(curHashNode, name2);
                    successful = correctNode;

                    // If variable is in symbol table, get its type
                    if (successful)
                    {
                        type2 = correctNode->getType(name2);
                        correctNode->setLineAndColumn(name2, linkedToken2->getLine(), linkedToken2->getColumn());
                        
                        // Set assigned variable to initialized
                        if (branchName == "Assignment Statement")
                        {
                            // Give warning if assigning variable to the same uninitialized variable 
                            if (name1 == name2 && !correctNode->checkInitialized(name1))
                            {
                                log("WARNING", "Initialized variable [" + name2 + "] with itself at (" + to_string(linkedToken1->getLine()) + ":" + to_string(linkedToken1->getColumn()) + ")");
                                warningCount++;
                            }
                        }
                        else
                        {
                            correctNode->setUsed(name2);

                            // Throw warning if it wasn't initialized yet
                            if (!correctNode->checkInitialized(name2))
                            {
                                log("WARNING", "Using uninitialized variable " + name2 + " at (" + to_string(linkedToken2->getLine()) + ":" + to_string(linkedToken2->getColumn()) + ")");
                            }
                        }
                    }
                    else
                    {
                        log("ERROR", "Use of undeclared variable '" + name2 + "' at (" + to_string(linkedToken2->getLine()) + ":" + to_string(linkedToken2->getColumn()) + ")");
                        errorCount++;
                        noErrors = false;
                    }
                }

                // Type checking
                if (noErrors)
                {
                    // Set assigned variable to initialized
                    if (branchName == "Assignment Statement" && name1 != "ADD")
                    {
                        correctNode->setInitialized(name1);
                    }

                    // Get type if it wasn't specified for the first ("ADD" and literals)
                    if (name1 == "ADD")
                    {
                        type1 = "int";
                        linkedToken1 = leaf1->getChild(0)->getToken();
                    }
                    else if (type1 == "UNKNOWN")
                    {
                        if (linkedToken1->getType() == "DIGIT")
                        {
                            type1 = "int";
                        }
                        else if (linkedToken1->getType() == "QUOTE")
                        {
                            type1 = "string";
                        }
                        else if (linkedToken1->getType() == "BOOL_VAL")
                        {
                            type1 = "boolean";
                        }
                    }

                    // Get type if it wasn't specified for the second ("ADD" and literals)
                    if (name2 == "ADD")
                    {
                        type2 = "int";
                        linkedToken2 = leaf2->getChild(0)->getToken();
                    }
                    else if (type2 == "UNKNOWN")
                    {
                        if (linkedToken2->getType() == "DIGIT")
                        {
                            type2 = "int";
                        }
                        else if (linkedToken2->getType() == "QUOTE")
                        {
                            type2 = "string";
                        }
                        else if (linkedToken2->getType() == "BOOL_VAL")
                        {
                            type2 = "boolean";
                        }
                    }

                    // If types don't match, throw an error
                    if (type1 != type2)
                    {
                        // Correct error messaging
                        string operation = "Comparing";
                        if (branchName == "Assignment Statement")
                        {
                            operation = "Assigning";
                        }

                        // This statement can only be true when in if/while
                        if (name1 == "ADD")
                        {
                            log("ERROR", "Type mismatch: " + operation + " " + type1 + " expression to " + type2 + " variable [" + name2 + "] at (" + to_string(linkedToken2->getLine()) + ":" + to_string(linkedToken2->getColumn()) + ")");
                        }
                        // This statement can be true in either assignment or if/while
                        else if (name2 == "ADD")
                        {
                            log("ERROR", "Type mismatch: " + operation + " " + type1 + " variable [" + name1 + "] to " + type2 + " expression at (" + to_string(linkedToken1->getLine()) + ":" + to_string(linkedToken1->getColumn()) + ")");
                        }
                        // Type mismatch error when both are IDs
                        else if (linkedToken1->getType() == "ID" && linkedToken2->getType() == "ID")
                        {
                            log("ERROR", "Type mismatch: " + operation + " " + type1 + " variable [" + name1 + "] to " + type2 + " variable [" + name2 + "] at (" + to_string(linkedToken1->getLine()) + ":" + to_string(linkedToken1->getColumn()) + ")");
                        }
                        // Type mismatch error when dealing with first ID and second literal
                        else if (linkedToken1->getType() == "ID")
                        {
                            log("ERROR", "Type mismatch: " + operation + " " + type1 + " variable [" + name1 + "] to " + type2 + " literal [" + name2 + "] at (" + to_string(linkedToken1->getLine()) + ":" + to_string(linkedToken1->getColumn()) + ")");
                        }
                        // Type mismatch error when dealing with first literal and second ID
                        else if (linkedToken1->getType() == "ID")
                        {
                            log("ERROR", "Type mismatch: " + operation + " " + type1 + " literal [" + name1 + "] to " + type2 + " variable [" + name2 + "] at (" + to_string(linkedToken1->getLine()) + ":" + to_string(linkedToken1->getColumn()) + ")");
                        }
                        // Type mismatch error when dealing with two literals
                        else
                        {
                            log("ERROR", "Type mismatch: " + operation + " " + type1 + " literal [" + name1 + "] to " + type2 + " literal [" + name2 + "] at (" + to_string(linkedToken1->getLine()) + ":" + to_string(linkedToken1->getColumn()) + ")");
                        }
                        
                        
                        errorCount++;
                        noErrors = false;
                    }
                }
            }
            // Declare variable
            else if (branchName == "Var Decl")
            {
                // Get the variable name and its type
                string type = curBranch->getChild(0)->getName();
                string var = curBranch->getChild(1)->getName();

                // Add the hash value
                successful = curHashNode->addValue(var, type);

                linkedToken1 = curBranch->getChild(0)->getToken();

                // If no collision, add line and column members to hash
                if (successful)
                {
                    curHashNode->setLineAndColumn(var, linkedToken1->getLine(), linkedToken1->getColumn());
                }
                // If there was a collision, throw error
                else
                {
                    log("ERROR", "Redeclared identifier " + var + " at (" + to_string(linkedToken1->getLine()) + ":" + to_string(linkedToken1->getColumn()) + ")");
                    errorCount++;
                }
            }
        }

        // Does semantic checking when involving ADD statements
        void checkWithADDs(Node* leaf, HashNode* hashNode, Node* overallBranch)
        {
            string name = leaf->getName();
            bool isCorrect = false;
            Token* linkedToken;
            HashNode* correctNode;
            string overallBranchName = overallBranch->getName();

            // Gets the linked token if it is a leaf (and not another ADD branch)
            if (leaf->isTokenLinked())
            {
                linkedToken = leaf->getToken();
            }

            // Don't check further ADD blocks since those have already been evaluated
            if (name == "ADD")
            {
                isCorrect = true;
            }
            // Checks if its a variable
            if (name != "ADD" && leaf->isTokenLinked() && linkedToken->getType() == "ID")
            {
                correctNode = findInSymbolTable(hashNode, name);
                isCorrect = correctNode;
            }
            // Checks if its a digit
            else if (leaf->isTokenLinked() && linkedToken->getType() == "DIGIT")
            {
                isCorrect = true;
            }
            
            // If not successful, error/warning based on overarching statement
            if (!isCorrect)
            {
                // These statements cannot use undeclared variables  
                if (overallBranchName == "Print Statement" || overallBranchName == "Assignment Statement" ||
                    overallBranchName == "isEq" || overallBranchName == "isNotEq")
                {
                    if (leaf->isTokenLinked() && linkedToken->getType() == "ID")
                    {
                        log("ERROR", "Use of undeclared variable '" + name + "' at (" + to_string(linkedToken->getLine()) + ":" + to_string(linkedToken->getColumn()) + ")");
                    }
                    // If variable is declared, then there was a type error
                    else
                    {
                        string type;
                        if (linkedToken->getType() == "QUOTE")
                        {
                            type = "string";
                        }
                        else if (linkedToken->getType() == "BOOL_VAL")
                        {
                            type = "boolean";
                        }
                        log("ERROR", "Using " + type + " in int expression at (" + to_string(linkedToken->getLine()) + ":" + to_string(linkedToken->getColumn()) + ")");
                    }
                    errorCount++;
                }
            }
            // If it was correct and a variable
            else if (leaf->isTokenLinked() && linkedToken->getType() == "ID")
            {
                // If the variable involved isn't an integer type, throw type error
                if (correctNode->getType(name) != "int")
                {
                    log("ERROR", "Type mismatch: Using " + correctNode->getType(name) + " variable [" + name + "] in int expression at (" + to_string(linkedToken->getLine()) + ":" + to_string(linkedToken->getColumn()) + ")");
                    errorCount++;
                }
                // Set variable to used
                if (overallBranchName == "Print Statement" || overallBranchName == "isEq" || overallBranchName == "isNotEq")
                {
                    correctNode->setUsed(name);

                    // Throw warning if it wasn't initialized yet
                    if (!correctNode->checkInitialized(name))
                    {
                        log("WARNING", "Using uninitialized variable " + name + " at (" + to_string(linkedToken->getLine()) + ":" + to_string(linkedToken->getColumn()) + ")");
                    }
                }
                // Check if using the same variable to initialize an uninitialized variable
                else if (overallBranchName == "Assignment Statement")
                {
                    string overallBranchChildName = overallBranch->getChild(0)->getName();

                    // Give warning if assigning variable to the same uninitialized variable 
                    if (name == overallBranchChildName && !correctNode->checkInitialized(overallBranchChildName))
                    {
                        log("WARNING", "Initialized variable [" + name + "] with itself at (" + to_string(linkedToken->getLine()) + ":" + to_string(linkedToken->getColumn()) + ")");
                        warningCount++;
                    }
                }
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
            //warningCount += node->warningCheck();

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