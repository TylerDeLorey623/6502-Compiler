#ifndef SYMBOL_H
#define SYMBOL_H

#include <unordered_map>

using namespace std;

// All types of tokens for this compiler
enum class Symbol
{
    CURLYBRACE_OPEN, CURLYBRACE_CLOSE, PRINT, WHILE, IF, DOUBLE_QUOTE, 
    PARENTHESIS_OPEN, PARENTHESIS_CLOSE, TYPE, CHAR, DIGIT, EQUALITY, 
    INEQUALITY, FALSE, TRUE, ADDITION, ASSIGNMENT, EOP, UNKNOWN
};


Symbol getSymbol(char targetChar)
{
    Symbol symbolReturn;

    static const unordered_map<char, Symbol> symbolMap = 
    {
        {'{', Symbol::CURLYBRACE_OPEN},
        {'}', Symbol::CURLYBRACE_CLOSE},
        {'"', Symbol::DOUBLE_QUOTE},
        {'(', Symbol::PARENTHESIS_OPEN},
        {')', Symbol::PARENTHESIS_CLOSE},
        {'+', Symbol::ADDITION},
        {'=', Symbol::ASSIGNMENT},
        {'$', Symbol::EOP}
    };

    auto target = symbolMap.find(targetChar);
    if (target != symbolMap.end())
    {
        symbolReturn = target->second;
    }
    else
    {
        symbolReturn = Symbol::UNKNOWN;
    }

    return symbolReturn;
}

// Converts Symbol to a string that can be printed
string symbolToString(Symbol symbol) {
    switch (symbol) 
    {
        case Symbol::CURLYBRACE_OPEN: 
            return "CURLYBRACE_OPEN";
        case Symbol::CURLYBRACE_CLOSE: 
            return "CURLYBRACE_CLOSE";
        case Symbol::PRINT: 
            return "PRINT";
        case Symbol::WHILE: 
            return "WHILE";
        case Symbol::IF: 
            return "IF";
        case Symbol::DOUBLE_QUOTE: 
            return "DOUBLE_QUOTE";
        case Symbol::PARENTHESIS_OPEN: 
            return "PARENTHESIS_OPEN";
        case Symbol::PARENTHESIS_CLOSE: 
            return "PARENTHESIS_CLOSE";
        case Symbol::TYPE: 
            return "TYPE";
        case Symbol::CHAR: 
            return "CHAR";
        case Symbol::DIGIT: 
            return "DIGIT";
        case Symbol::EQUALITY: 
            return "EQUALITY";
        case Symbol::INEQUALITY: 
            return "INEQUALITY";
        case Symbol::FALSE: 
            return "FALSE";
        case Symbol::TRUE: 
            return "TRUE";
        case Symbol::ADDITION: 
            return "ADDITION";
        case Symbol::ASSIGNMENT: 
            return "ASSIGNMENT";
        case Symbol::EOP: 
            return "EOP";
        default: 
            return "UNKNOWN";
    }
}

#endif