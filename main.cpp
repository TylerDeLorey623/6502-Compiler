#include <iostream>
#include <fstream>
#include <vector>

#include "Token.h"
#include "Symbol.h"
#include "Keyword.h"

using namespace std;

int main() 
{
    // Variable declaration
    
    
    // File IO
    ifstream file("code.txt");
    if (!file)
    {
        cout << "File failed to open." << endl;
        return 1;
    }

    // Copies all characters from the file to the code string
    string code((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

    //cout << "DEBUG Lexer - " << savedItem << " [ " <<  savedBuffer << " ] found at (" << savedItemLine << ":" << savedItemColumn << ")" << endl;

    // Close file
    file.close();

}