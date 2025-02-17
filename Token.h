#ifndef TOKEN_H
#define TOKEN_H

#include <iostream>

using namespace std;

class Token
{
    public:
        string type;
        string lexeme;
        int line;
        int column;

        Token(string newType, string newLexeme, int newLine, int newColumn)
        {
            this->type = newType;
            this->lexeme = newLexeme;
            this->line = line;
            this->column = newColumn;
        }

};

#endif