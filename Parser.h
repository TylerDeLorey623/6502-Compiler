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
            if (currentToken->getType() == expectedTokenType)
            {
                // CONSUME
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

        void parseProgram()
        {
            log("DEBUG", "parseProgram()");
            parseBlock();
            match("EOP");
        }

        void parseBlock()
        {
            log("DEBUG", "parseBlock()");
            match("OPEN_CURLY");
            parseStatementList();
            match("CLOSE_CURLY");
        }

        void parseStatementList()
        {
            log("DEBUG", "parseStatementList()");
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
            log("DEBUG", "parseStatement()");
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
            log("DEBUG", "parsePrintStatement()");
            match("PRINT_STATEMENT");
            match("OPEN_PARENTHESIS");
            parseExpr();
            match("CLOSE_PARENTHESIS");
        }

        void parseAssignmentStatement()
        {
            log("DEBUG", "parseAssignmentStatement()");
            parseId();
            match("ASSIGNMENT_OP");
            parseExpr();
        }

        void parseVarDecl()
        {
            log("DEBUG", "parseVarDecl()");
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
            log("DEBUG", "parseWhileStatement()");
            match("WHILE_STATEMENT");
            parseBooleanExpr();
            parseBlock();
        }

        void parseIfStatement()
        {
            log("DEBUG", "parseIfStatement()");
            match("IF_STATEMENT");
            parseBooleanExpr();
            parseBlock();
        }

        void parseExpr()
        {
            log("DEBUG", "parseExpr()");
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
            log("DEBUG", "parseIntExpr()");
            match("DIGIT");
            if (currentTokenType == "ADDITION_OP")
            {
                match("ADDITION_OP");
                parseExpr();
            }
        }

        void parseStringExpr()
        {
            log("DEBUG", "parseStringExpr()");
            match("QUOTE");
            parseCharList();
            match("QUOTE");
        }

        void parseBooleanExpr()
        {
            log("DEBUG", "parseBooleanExpr()");
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
            log("DEBUG", "parseId()");
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
            log("DEBUG", "parseCharList()");
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