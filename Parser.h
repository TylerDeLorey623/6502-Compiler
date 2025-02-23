#ifndef PARSER_H
#define PARSER_H

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

            // Records size of the Token vector
            this->size = tokens.size();

            // Stores address of the current Token and its type
            this->currentToken = &tokens[currentIndex]; 
            this->currentTokenType = currentToken->type;
        }

        // Validates the tokens
        void parse()
        {
            parseProgram();
        }

    private:
        // Default members
        int programNumber;
        vector<Token> tokens;
        string delimiter;
        int size;

        // Current token, its type, and index in the tokens vector
        Token* currentToken = nullptr;
        string currentTokenType;
        int currentIndex = 0;

        int errorCount = 0;
        int warningCount = 0;

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

        void match(string expectedTokenType)
        {
            if (currentToken->type == expectedTokenType)
            {
                // CONSUME
                currentIndex++;

                // Check if index is at the end of the Tokens
                if (currentIndex < size)
                {
                    currentToken = &tokens[currentIndex];
                    currentTokenType = currentToken->type;
                }
                else
                {
                    currentToken = nullptr;
                    currentTokenType = "";
                }
            }
            else
            { 
                const string errMessage = "EXPECTED [" + expectedTokenType + "] BUT FOUND [" + currentTokenType + "] with value '" + currentToken->lexeme + "' at (" + to_string(currentToken->line) + ":" + to_string(currentToken->column) + ")"; 
                log("ERROR", errMessage);
            }
        }

        void parseProgram()
        {
            parseBlock();
            match("EOP");
        }

        void parseBlock()
        {
            match("OPEN_CURLY");
            parseStatementList();
            match("CLOSE_CURLY");
        }

        void parseStatementList()
        {
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
        }

        void parseStatement()
        {
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
        }

        void parsePrintStatement()
        {
            match("PRINT_STATEMENT");
            match("OPEN_PARENTHESIS");
            parseExpr();
            match("CLOSE_PARENTHESIS");
        }

        void parseAssignmentStatement()
        {
            parseId();
            match("ASSIGNMENT_OP");
            parseExpr();
        }

        void parseVarDecl()
        {
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
        }

        void parseWhileStatement()
        {
            match("WHILE_STATEMENT");
            parseBooleanExpr();
            parseBlock();
        }

        void parseIfStatement()
        {
            match("IF_STATEMENT");
            parseBooleanExpr();
            parseBlock();
        }

        void parseExpr()
        {
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
        }

        void parseIntExpr()
        {
            match("DIGIT");
            if (currentTokenType == "ADDITION_OP")
            {
                match("ADDITION_OP");
                parseExpr();
            }
        }

        void parseStringExpr()
        {
            match("QUOTE");
            parseCharList();
            match("QUOTE");
        }

        void parseBooleanExpr()
        {
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
            }
            // Will give error if type isn't a boolean value
            else
            {
                match("BOOL_VAL");
            }
        }

        void parseId()
        {
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
        }

        void parseCharList()
        {
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
        }
};

#endif