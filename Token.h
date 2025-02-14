#ifndef TOKEN_H
#define TOKEN_H

#include <iostream>

using namespace std;

class Token
{
    public:
        string type;
        string lexeme;
        int row;
        int column;

        Token(string newType, string newLexeme, int newRow, int newColumn)
        {
            this->type = newType;
            this->lexeme = newLexeme;
            this->row = newRow;
            this->column = newColumn;
        }

};

#endif