#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>

#include "Token.h"
#include "Lexer.h"

using namespace std;

// Splits up the programs by the delimiter
vector<string> splitPrograms(const string& input, const char delimiter)
{
    const regex commentBeginREGEX = regex(R"(^\/\*)");
    const regex commentEndREGEX = regex(R"(^\*\/)");

    vector<string> segments;
    string str = "";
    bool insideComment = false;
    
    for (int currentPos = 0, size = input.size(); currentPos < size; currentPos++)
    {
        // Detect comments (delimiter in comments will not separate program)
        string snippet = input.substr(currentPos);
        smatch match;
        if (regex_search(snippet, match, commentBeginREGEX))
        {
            insideComment = true;
        }
        else if (regex_search(snippet, match, commentEndREGEX))
        {
            insideComment = false;
        }

        // Add character to current program
        str += input[currentPos];

        // If that character is the delimiter, add a segment and start creating a new string
        if (input[currentPos] == delimiter && !insideComment)
        {
            segments.emplace_back(str);
            str = "";
        }
    }  

    // Test if last character is the delimiter for warning error
    if (input.back() != delimiter)
    {
        segments.emplace_back(str);
        cout << "WARNING: The final program does not end with a '" << delimiter << "'. Temporarily added the EOP for the compilation process." << endl << endl;
    }

    return segments;
}


int main(int argc, char* argv[]) 
{
    // File IO
    if (argc != 2)
    {
        cout << "Arguments were incorrect. Use command 'make ARGS='filename''." << endl;
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

    // Close file
    file.close();

    // Remove trailing whitespace
    code.erase(code.find_last_not_of(" \t\n\r\f\v") + 1);

    // Vector that stores programs separated with $
    vector<string> programs = splitPrograms(code, '$');

    // Compile each program
    for (int i = 0, size = programs.size(); i < size; i++)
    {
        // LEXER
        cout << "INFO  Lexer - Lexing program " << i + 1 << "..." << endl;
        Lexer currentLex = Lexer(programs[i]);
        auto result = currentLex.tokenize();

        vector<Token> tokens = result.first;
        int errors = result.second;

        cout << "INFO  Lexer - Lex completed with " << errors << " error(s)" << endl;

        // PARSER
        // SEMANATIC ANALYSIS
        // CODE GEN
        cout << endl;
    }

}