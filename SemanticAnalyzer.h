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

        // Converting strings into a single leaf node
        bool inQuotes = false;
        string currentString;

        // IF/WHILE branch
        bool collectNodes = false;
        vector<string> nodeNames;
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
                        formatNodes();
                    }
                    // Add the branch node to the AST (follow CST structure)
                    else
                    {
                        myAST->addNode("branch", bName);

                        // If its a block, move up scope pointer
                        if (bName == "Block")
                        {
                            currentScope++;
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
            // Gets current branch information
            Node* curBranch = myAST->getCurrentBranch();
            string branchName = curBranch->getName();

            if (branchName == "Block")
            {
                currentScope--;
            }

            cout << "Scope: " << currentScope << endl;
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
                    continue;
                }
                // If its a regular value, add a leaf node
                else if (name != "==" && name != "!=")
                {
                    myAST->addNode("leaf", name);
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