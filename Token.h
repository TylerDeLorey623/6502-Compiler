#ifndef TOKEN_H
#define TOKEN_H

#include <iostream>

#include "Symbol.h"

using namespace std;

class Token
{
    public:
        Symbol type;
        string lexeme;
        int line;
        int column;

        Token(Symbol newType, string newLexeme, int newLine, int newColumn)
        {
            this->type = newType;
            this->lexeme = newLexeme;
            this->line = newLine;
            this->column = newColumn;
        }

};

#endif