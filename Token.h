#ifndef TOKEN_H
#define TOKEN_H

#include <iostream>

using namespace std;

class Token
{
    public:
        // Parameterized Constructor
        Token(string newType, string newLexeme, int newLine, int newColumn)
        {
            this->type = newType;
            this->lexeme = newLexeme;
            this->line = newLine;
            this->column = newColumn;
        }

        // Empty constructor (default in Parser class, gets rewritten)
        Token()
        {
            this->type = "UNKNOWN";
            this->lexeme = "UNKNOWN";
            this->line = 0;
            this->column = 0;
        }

        // Getters
        string getType()
        {
            return this->type;
        }

        string getLexeme()
        {
            return this->lexeme;
        }

        int getLine()
        {
            return this->line;
        }

        int getColumn()
        {
            return this->column;
        }

    private:
        // Members of Token Class
        string type;
        string lexeme;
        int line;
        int column;
};

#endif