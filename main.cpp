#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>

#include "Token.h"
#include "Lexer.h"

using namespace std;

// Splits up the programs by the delimiter
vector<string> splitPrograms(const string input, const char delimiter)
{
    vector<string> segments;
    stringstream stream(input);
    string str;

    while (getline(stream, str, delimiter))
    {
        segments.emplace_back(str);
    }

    // Test if last character is the delimiter for warning error
    if (input.back() != delimiter)
    {
        cout << "WARNING: The final program does not end with a '" << delimiter << "'." << endl;
    }

    return segments;
}


int main(int argc, char* argv[]) 
{
    // File IO
    if (argc != 2)
    {
        cout << "No arguments given. Use command 'make ARGS='filename''." << endl;
        return 1;
    }

    ifstream file(argv[1]);
    if (!file)
    {
        cout << "File failed to open." << endl;
        return 1;
    }

    // Copies all characters from the file to the code string
    string code((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

    // Create the separate programs separated with $
    vector<string> programs = splitPrograms(code, '$');

    // Compile each program
    for (int i = 0, size = programs.size(); i < size; i++)
    {
        cout << "INFO  Lexer - Lexing program " << i + 1 << "..." << endl;
        Lexer* currentLex = new Lexer(programs[i]);
        auto result = currentLex->tokenize();

        vector<Token> tokens = result.first;
        int errors = result.second;

        cout << "INFO  Lexer - Lex completed with " << errors << " error(s)" << endl;
        // PARSER
        // SEMANATIC ANALYSIS
        // CODE GEN
        delete(currentLex);
        cout << endl;
    }

    //cout << "DEBUG Lexer - " << savedItem << " [ " <<  savedBuffer << " ] found at (" << savedItemLine << ":" << savedItemColumn << ")" << endl;

    // Close file
    file.close();

}