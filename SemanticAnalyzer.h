#ifndef ANALYZER_H
#define ANALYZER_H

#include "Tree.h"

using namespace std;

// The Semantic Analysis Class
class SemanticAnalyzer
{
    public:
        // Default constructor for the Analysis class
        SemanticAnalyzer(const int progNum, const vector<Token>& lexTokens)
        {
            this->programNumber = progNum;
            this->tokens = lexTokens;
            
            this->size = tokens.size();
            
            // Creates a pointer for the AST
            myAST = new Tree();

            // Stores address of the current Token and its type
            this->currentToken = &tokens[currentIndex]; 
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

        // Generates the AST based on token stream by doing a modified version of the Recursive Decent Parser and grabbing the important things
        // This time there will be no error checking since it is all already correct 
        void generateAST()
        {
            return;
        }

    private:
        // Default members
        int programNumber;
        vector<Token> tokens;

        int size;

        // AST Members
        Tree* myAST;
        string traversalResult;

        // Current token and index in the tokens vector
        Token* currentToken = nullptr;
        int currentIndex = 0;

        int errorCount = 0;
        int warningCount = 0;

        // Deletes AST from Memory by deleting its Nodes recursively
        void deleteNode(Node* curNode)
        {
            vector<Node*> nodeChildren = curNode->getChildren(); 
            for (int i = 0, n = nodeChildren.size(); i < n; i++)
            {
                deleteNode(nodeChildren[i]);
            }
            delete(curNode);
        }

        // Most of the expand() function references code by Alan G. Labouseur, based on the 2009 work by Michael Ardizzone and Tim Smith.
        // Recursive function to handle the expansion of the nodes
        void expand(Node* curNode, int depth)
        {
            // Space out based on the current depth so
            // this looks at least a little tree-like.
            for (int i = 0; i < depth; i++)
            {
                traversalResult += "-";
            }

            // If it was linked to a token, that means its a leaf node
            if (curNode->isTokenLinked())
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

        // Matches the expected token to find to the current token
        void match(string expectedTokenType)
        {
            // Adds the leaf node and links the token to this Node
            myAST->addNode("leaf", currentToken->getLexeme());
            myAST->getMostRecentNode()->linkToken(currentToken);

            currentIndex++;

            // Check if index is at the end of the Tokens
            if (currentIndex < size)
            {
                currentToken = &tokens[currentIndex];
            }
            else
            {
                currentToken = nullptr;
            }
        }
};

#endif