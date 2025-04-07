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
            // Initialize the result string
            traversalResult = "";

            // Make initial call to expand from root
            expand(myAST->getRoot(), 0);

            // Print results
            cout << traversalResult << endl;
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
            log("INFO", "AST for Program #" + to_string(programNumber));
            inorder(programCST->getRoot(), 0);
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

        // Converting strings into a single leaf node
        bool inQuotes = false;
        string currentString;

        // IF/WHILE branch
        bool collectNodes = false;
        vector<string> nodeNames;
        vector<Token*> tokenCollect;
        int oldDepth = 0;
        string equalitySign = "UNKNOWN";

        // ADD branch
        int add = 0;
        int ifWhileAdd = 0;

        // For leaf nodes
        Token* currentToken;
        Token* heldToken;

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
        void inorder(Node *node, int depth)
        {
            // If linked, is a leaf node, add if important
            if (node->isTokenLinked())
            {
                // Check if leaf is important
                string lName = node->getName();
                if (lName != "{" && lName != "}" && lName != "print" && lName != "while" && lName != "if" && lName != "(" && 
                    lName != ")" && lName != "=" && lName != "$" && lName != "+")
                {
                    // Get the linked token
                    currentToken = node->getToken(); 

                    // Concatenate characters to make a string if looking at chars
                    if (lName == "\"") 
                    {
                        // Toggle quote state
                        inQuotes = !inQuotes;
                        
                        // If just got out of quotes, add the string to the Tree and link the held token
                        if (!inQuotes && !collectNodes) 
                        {
                            myAST->addNode("leaf", currentString);
                            myAST->getMostRecentNode()->linkToken(heldToken);
                        } 
                        // Collect node if they need to be collected for if and while statement
                        else if (!inQuotes && collectNodes)
                        {
                            nodeNames.emplace_back(currentString);
                            tokenCollect.emplace_back(currentToken);
                        }
                        // If just got into the quotes, clear the string that will be overwritten and get token that will be linked
                        else 
                        {
                            heldToken = currentToken;
                            currentString.clear();
                        }
                    }
                    // If in quotes, add to the string, don't add a new Node 
                    else if (inQuotes) 
                    {
                        currentString += lName;
                    }
                    // Stop collecting nodes if its just a if/while statement with either true/false (not an expression)
                    else if (collectNodes && (lName == "true" || lName == "false"))
                    {
                        collectNodes = false;
                        nodeNames.clear();
                        myAST->addNode("leaf", lName);
                        myAST->getMostRecentNode()->linkToken(currentToken);
                    }
                    // Collect node if they need to be collected for if and while statement
                    else if (collectNodes)
                    {
                        // Note equality sign if there is one
                        if (lName == "==")
                        {
                            equalitySign = "isEq";
                        }
                        else if (lName == "!=")
                        {
                            equalitySign = "isNotEq";
                        }

                        nodeNames.emplace_back(lName);
                        tokenCollect.emplace_back(currentToken);
                    }
                    // Base case: Add the leaf node to the tree and link token
                    else 
                    {
                        myAST->addNode("leaf", lName);
                        myAST->getMostRecentNode()->linkToken(currentToken);
                    }
                }
            }
            // If not linked, is a branch node, add if important
            else
            {
                // Check if branch is important
                bool important = false;
                string bName = node->getName();
                if (bName == "Block" || bName == "Print Statement" || bName == "Assignment Statement" || bName == "Var Decl" ||
                    bName == "While Statement" || bName == "If Statement")
                {
                    important = true;

                    // Tree structure is different for IF and WHILE statements, start collecting nodes to add to vector
                    if (!collectNodes && (bName == "While Statement" || bName == "If Statement"))
                    {
                        collectNodes = true;
                        oldDepth = depth;

                        myAST->addNode("branch", bName);
                    }
                    // If in collecting mode and the statement is finished, stop collecting and add nodes in proper order
                    else if (collectNodes && oldDepth == depth - 1)
                    {
                        collectNodes = false;
                        nodeNames.emplace_back(bName);
                        tokenCollect.emplace_back(new Token());
                        formatNodes();
                    }
                    // Add the branch node to the AST (follow CST structure)
                    else
                    {
                        myAST->addNode("branch", bName);

                        // If its a block, create a new node on the symbol table
                        if (bName == "Block")
                        {
                            currentScope++;
                            mySym->addHashNode("Scope " + to_string(currentScope) + getScopeSubValue(currentScope));
                        }
                    }
                }
                // Tree structure is different for ADD blocks
                else if (bName == "Int Expr" && node->getChildren().size() > 1)
                {
                    // If in collecting mode, collect the node (add a branch if not)
                    if (collectNodes)
                    {
                        nodeNames.emplace_back("ADD");
                        tokenCollect.emplace_back(new Token());
                    }
                    else
                    {
                        // Add an add branch
                        add++;
                        myAST->addNode("branch", "ADD");
                    }
                }

                // Recursively expand the branches
                for (int i = 0, childrenSize = node->getChildren().size(); i < childrenSize; i++)
                {
                    inorder(node->getChild(i), depth + 1);
                }

                // Move up if the branch was important
                if (important)
                {
                    important = false;

                    // Move up more if it was an ADD branch
                    while (add > 0)
                    {
                        add--;
                        symbolAndMove();
                    }

                    symbolAndMove();
                }
            }
        }

        // Two functions are called whenever moving branch up
        // First deals with symbol table scope/type checking
        // Second deals with moving current node up the AST  
        void symbolAndMove()
        {
            semanticCheck();
            myAST->moveUp();
        }

        // Does scope/type checking on the current branch
        void semanticCheck()
        {
            // Gets current branch information and its children
            Node* curBranch = myAST->getCurrentBranch();
            string branchName = curBranch->getName();
            Node* leaf1;
            Node* leaf2;
            Token* linkedToken;
            bool successful = false;
            string name;

            // Move up tree if scope was changed
            if (branchName == "Block")
            {
                mySym->moveUp();
                currentScope--;
            }

            // Get information about the current HashNode
            HashNode* curHashNode = mySym->getCurrentHashNode();

            // Scope/type checking for each type of statement
            if (branchName == "Print Statement")
            {
                // Set the variable to used if it exists
                leaf1 = curBranch->getChild(0);
                name = leaf1->getName();
                linkedToken = leaf1->getToken();

                // Finds the variable (if it exists)
                HashNode* initialNode = curHashNode;
                while (!successful && initialNode == nullptr)
                {
                    curHashNode = initialNode;
                    successful = curHashNode->exists(name);
                    initialNode = curHashNode->getParent();
                }

                // Sets the variable to used if it was found
                if (successful)
                {
                    curHashNode->setUsed(name);
                }
                else
                {
                    log("ERROR", "Use of undeclared variable '" + name + "' at (" + to_string(linkedToken->getLine()) + ":" + to_string(linkedToken->getColumn()) + ")");
                    errorCount++;
                }
            }
            else if (branchName == "Assignment Statement")
            {
                
            }

            if (!successful)
            {
                cout << "NOT SUCCESSFUL" << endl;
            }

            cout << curHashNode->getName() << endl;
            cout << branchName << endl << endl;
        }

        // Format the nodes from the IF/WHILE statements
        void formatNodes()
        {
            myAST->addNode("branch", equalitySign);

            // Iterate through stored nodes
            for (int i = 0; i < nodeNames.size(); i++)
            {
                string name = nodeNames[i];
                Token* curToken = tokenCollect[i];
                
                // If its an add, add a new branch
                if (name == "ADD")
                {
                    ifWhileAdd++;
                    myAST->addNode("branch", name);
                }
                // If it was a Block, nodeNames should be completed, move up tree, add node, and exit loop
                else if (name == "Block")
                {
                    // Move up tree more if there was ADD
                    while (ifWhileAdd > 0)
                    {
                        ifWhileAdd--;
                        symbolAndMove();
                    }
                    symbolAndMove();
                    myAST->addNode("branch", name);

                    // Change scope
                    currentScope++;
                    mySym->addHashNode("Scope " + to_string(currentScope) + getScopeSubValue(currentScope));
                    continue;
                }
                // If its a regular value, add a leaf node
                else if (name != "==" && name != "!=")
                {
                    myAST->addNode("leaf", name);
                    myAST->getMostRecentNode()->linkToken(curToken);
                }
                // If its an equality sign, move up the tree if there were ADDs and continue
                else
                {
                    while (ifWhileAdd > 0)
                    {
                        ifWhileAdd--;
                        symbolAndMove();
                    }
                }
            }

            // Reset nodes
            equalitySign = "UNKNOWN";
            nodeNames.clear();

            // Delete all extra unnecessary Tokens and clear the Token List
            for (int i = 0, n = tokenCollect.size(); i < n; i++)
            {
                if (tokenCollect[i]->getType() == "UNKNOWN")
                {
                    delete(tokenCollect[i]);
                }
            }
            tokenCollect.clear();
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