#ifndef PARSER_H
#define PARSER_H

#include "CST.h"

using namespace std;

// The Parser Class
class Parser
{
    public:
        // Default constructor for the Parser class
        Parser(const int progNum, const vector<Token> lexTokens, const char del)
        {
            this->programNumber = progNum;
            this->tokens = lexTokens; 
            this->delimiter = del;
            
            // Creates a pointer for the CST
            myCST = new CST();

            // Records size of the Token vector
            this->size = tokens.size();

            // Stores address of the current Token and its type
            this->currentToken = &tokens[currentIndex]; 
            this->currentTokenType = currentToken->getType();
        }

        // Validates the tokens
        void parse()
        {
            log("INFO", "Parsing Program #" + to_string(programNumber));
            log("DEBUG", "parse()");
            parseProgram();
            log("INFO", "Parse completed with " + to_string(errorCount) + " error(s) and " + to_string(warningCount) + " warning(s)");
        }

        // Prints the CST
        void generateCST()
        {
            // Initialize the result string
            traversalResult = "";

            // Make initial call to expand from root
            expand(myCST->getRoot(), 0);

            // Print results
            cout << traversalResult << endl;
        }

        // Deletes CST from Memory so there are no memory leaks
        void deleteCST()
        {
            deleteNode(myCST->getRoot());
            delete(myCST);
        }

        // Returns error count to see whether or not to continue with the CST
        int getErrors()
        {
            return errorCount;
        }

    private:
        // Default members
        int programNumber;
        vector<Token> tokens;
        string delimiter;
        int size;

        // CST Members
        CST* myCST;
        string traversalResult;

        // Current token, its type, and index in the tokens vector
        Token* currentToken = nullptr;
        string currentTokenType;
        int currentIndex = 0;

        int errorCount = 0;
        int warningCount = 0;

        // Deletes CST from Memory by deleting its Nodes recursively
        void deleteNode(CSTnode* curNode)
        {
            vector<CSTnode*> nodeChildren = curNode->getChildren(); 
            for (int i = 0, n = nodeChildren.size(); i < n; i++)
            {
                deleteNode(nodeChildren[i]);
            }
            delete(curNode);
        }

        // This expand() function references code by Alan G. Labouseur, based on the 2009 work by Michael Ardizzone and Tim Smith.
        // Recursive function to handle the expansion of the nodes
        void expand(CSTnode* curNode, int depth)
        {
            // Space out based on the current depth so
            // this looks at least a little tree-like.
            for (int i = 0; i < depth; i++)
            {
                traversalResult += "-";
            }

            // If there are no children (i.e., leaf nodes)...
            if (curNode->getChildren().empty())
            {
                // note the leaf node
                traversalResult += "[" + curNode->getName() + "] \n";
            }
            else
            {
                // There are children, so note these branch nodes
                traversalResult += "<" + curNode->getName() + "> \n";

                // Recursively expand the branches
                for (int i = 0, childrenSize = curNode->getChildren().size(); i < childrenSize; i++)
                {
                    expand(curNode->getChild(i), depth + 1);
                }
            }
        }

        // Logging function for Parser
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
                cout << "Parser - ";

                cout << message << endl;
            }
        }

        // Matches the current terminal to the current token
        void match(string expectedTokenType)
        {
            if (currentTokenType == expectedTokenType)
            {
                // Adds the leaf node and links the token to this Node
                myCST->addNode("leaf", currentToken->getLexeme());
                myCST->getMostRecentNode()->linkToken(currentToken);

                currentIndex++;

                // Check if index is at the end of the Tokens
                if (currentIndex < size)
                {
                    currentToken = &tokens[currentIndex];
                    currentTokenType = currentToken->getType();
                }
                else
                {
                    currentToken = nullptr;
                    currentTokenType = "";
                }
            }
            else
            {  
                log("ERROR", "EXPECTED [" + expectedTokenType + "] BUT FOUND [" + currentTokenType + "] with value '" + currentToken->getLexeme() + "' at (" + to_string(currentToken->getLine()) + ":" + to_string(currentToken->getColumn()) + ")");
                errorCount++;
            }
        }

        // THE FOLLOWING FUNCTIONS ARE FOR PARSING EACH NONTERMINAL IN THE ENTIRE GRAMMAR
        void parseProgram()
        {
            log("DEBUG", "Parsing Program...");
            myCST->addNode("root", "Program");
            parseBlock();
            match("EOP");
            myCST->moveUp();
        }

        void parseBlock()
        {
            log("DEBUG", "Parsing Block...");
            myCST->addNode("branch", "Block");
            match("OPEN_CURLY");
            parseStatementList();
            match("CLOSE_CURLY");
            myCST->moveUp();
        }

        void parseStatementList()
        {
            log("DEBUG", "Parsing Statement List...");
            myCST->addNode("branch", "Statement List");
            if (currentTokenType == "PRINT_STATEMENT" || currentTokenType == "ID" || 
                currentTokenType == "I_VARTYPE" || currentTokenType == "S_VARTYPE" || currentTokenType == "B_VARTYPE" ||
                currentTokenType == "WHILE_STATEMENT" || currentTokenType == "IF_STATEMENT" || currentTokenType == "OPEN_CURLY")
            {
                parseStatement();
                parseStatementList();
            }
            else
            {
                // nothing
                // its an ε
                // production
            }
            myCST->moveUp();
        }

        void parseStatement()
        {
            log("DEBUG", "Parsing Statement...");
            myCST->addNode("branch", "Statement");
            if (currentTokenType == "PRINT_STATEMENT")
            {
                parsePrintStatement();
            }
            else if (currentTokenType == "ID")
            {
                parseAssignmentStatement();
            }
            else if (currentTokenType == "I_VARTYPE" || currentTokenType == "S_VARTYPE" || currentTokenType == "B_VARTYPE")
            {
                parseVarDecl();
            }
            else if (currentTokenType == "WHILE_STATEMENT")
            {
                parseWhileStatement();
            }
            else if (currentTokenType == "IF_STATEMENT")
            {
                parseIfStatement();
            }
            // Will give error if the type isn't a open curly brace
            else
            {
                parseBlock();
            }
            myCST->moveUp();
        }

        void parsePrintStatement()
        {
            log("DEBUG", "Parsing Print Statement...");
            myCST->addNode("branch", "Print Statement");
            match("PRINT_STATEMENT");
            match("OPEN_PARENTHESIS");
            parseExpr();
            match("CLOSE_PARENTHESIS");
            myCST->moveUp();
        }

        void parseAssignmentStatement()
        {
            log("DEBUG", "Parsing Assignment Statement...");
            myCST->addNode("branch", "Assignment Statement");
            parseId();
            match("ASSIGNMENT_OP");
            parseExpr();
            myCST->moveUp();
        }

        void parseVarDecl()
        {
            log("DEBUG", "Parsing Var Decl...");
            myCST->addNode("branch", "Var Decl");
            if (currentTokenType == "I_VARTYPE")
            {
                match("I_VARTYPE");
                parseId();
            }
            else if (currentTokenType == "S_VARTYPE")
            {
                match("S_VARTYPE");
                parseId();
            }
            // Will give error if type isn't a boolean variable
            else
            {
                match("B_VARTYPE");
                parseId();
            }
            myCST->moveUp();
        }

        void parseWhileStatement()
        {
            log("DEBUG", "Parsing While Statement...");
            myCST->addNode("branch", "While Statement");
            match("WHILE_STATEMENT");
            parseBooleanExpr();
            parseBlock();
            myCST->moveUp();
        }

        void parseIfStatement()
        {
            log("DEBUG", "Parsing If Statement...");
            myCST->addNode("branch", "If Statement");
            match("IF_STATEMENT");
            parseBooleanExpr();
            parseBlock();
            myCST->moveUp();
        }

        void parseExpr()
        {
            log("DEBUG", "Parsing Expr...");
            myCST->addNode("branch", "Expr");
            if (currentTokenType == "DIGIT")
            {
                parseIntExpr();
            }
            else if (currentTokenType == "QUOTE")
            {
                parseStringExpr();
            }
            else if (currentTokenType == "OPEN_PARENTHESIS" || currentTokenType == "BOOL_VAL")
            {
                parseBooleanExpr();
            }
            // Will give error if the type isn't an ID
            else
            {
                parseId();
            }
            myCST->moveUp();
        }

        void parseIntExpr()
        {
            log("DEBUG", "Parsing Int Expr...");
            myCST->addNode("branch", "Int Expr");
            match("DIGIT");
            if (currentTokenType == "ADDITION_OP")
            {
                match("ADDITION_OP");
                parseExpr();
            }
            myCST->moveUp();
        }

        void parseStringExpr()
        {
            log("DEBUG", "Parsing String Expr...");
            myCST->addNode("branch", "String Expr");
            match("QUOTE");
            parseCharList();
            match("QUOTE");
            myCST->moveUp();
        }

        void parseBooleanExpr()
        {
            log("DEBUG", "Parsing Boolean Expr...");
            myCST->addNode("branch", "Boolean Expr");
            if (currentTokenType == "OPEN_PARENTHESIS")
            {
                match("OPEN_PARENTHESIS");
                parseExpr();
                if (currentTokenType == "EQUALITY_OP")
                {
                    match("EQUALITY_OP");
                }
                // Will give error if type isn't an inequality operator
                else
                {
                    match("INEQUALITY_OP");
                }
                parseExpr();
                match("CLOSE_PARENTHESIS");
            }
            // Will give error if type isn't a boolean value
            else
            {
                match("BOOL_VAL");
            }
            myCST->moveUp();
        }

        void parseId()
        {
            log("DEBUG", "Parsing Id...");
            myCST->addNode("branch", "Id");
            // Extra logic for difference between CHAR and ID
            if (currentTokenType == "CHAR")
            {
                match("CHAR");
            }
            // Will give error if type isn't an ID
            else
            {
                match("ID");
            }
            myCST->moveUp();
        }

        void parseCharList()
        {
            log("DEBUG", "Parsing Char List...");
            myCST->addNode("branch", "Char List");
            if (currentTokenType == "CHAR")
            {
                match("CHAR");
                parseCharList();
            }
            else if (currentTokenType == "SPACE")
            {
                match("SPACE");
                parseCharList();
            }
            else
            {
                // nothing
                // its an ε
                // production
            }
            myCST->moveUp();
        }
};

#endif