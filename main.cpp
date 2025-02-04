#include <iostream>
#include <fstream>
#include <vector>

#include "Token.h"
#include "Symbol.h"

using namespace std;

int main() 
{
    // Variable declaration
    char ch;
    string buffer = "";
    int lastPosition = -1;
    int currentPosition = 0;

    int currentSymbolLine = 1;
    int currentSymbolColumn = 1;
    int savedSymbolLine = 1;
    int savedSymbolColumn = 1;
    
    // File IO
    ifstream file;
    vector<char> lines;

    // Open file
    file.open("code.txt");
    if (!file.is_open())
    {
        cout << "File failed to open." << endl;
        return 1;
    }

    while (file.get(ch))
    {
        buffer += ch;

        if (ch == '\n')
        {
            currentSymbolLine++; 
            currentSymbolColumn = 0;   
        }

        Symbol sym = getSymbol(ch);
        if (sym != Symbol::UNKNOWN)
        {
            savedSymbolLine = currentSymbolLine;
            savedSymbolColumn = currentSymbolColumn;
            cout << "DEBUG Lexer - " << symbolToString(sym) << " [ " <<  ch << " ] found at (" << savedSymbolLine << ":" << savedSymbolColumn << ")" << endl;
        }

        currentSymbolColumn++;
        currentPosition++;
    }

    // Close file
    file.close();

}