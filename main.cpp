#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>

#include "Verbose.h"
#include "Token.h"

#include "Lexer.h"
#include "Parser.h"

using namespace std;

// Prototypes for functions in file
void log(const string type, const string message);
vector<string> splitPrograms(const string& input, const char delimiter);


int main(int argc, char* argv[]) 
{
    // File IO
    if (argc != 2)
    {
        cout << "Arguments were incorrect. Use command 'make FILE='filename''." << endl;
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
    char delimiter = '$';
    vector<string> programs = splitPrograms(code, delimiter);

    // Compile each program
    for (int i = 0, size = programs.size(); i < size; i++)
    {
        cout << endl;
        int errors = 0;
        
        // LEXER
        Lexer currentLex = Lexer(i + 1, programs[i], delimiter);
        auto lexResult = currentLex.tokenize();
        vector<Token> tokens = lexResult.first;
        errors = lexResult.second;

        cout << endl;

        // PARSER
        if (errors > 0)
        {
            log("INFO", "Parsing for Program #" + to_string(i + 1) + " skipped due to Lex error(s)");
            continue;
        }

        Parser currentParse = Parser(i + 1, tokens, delimiter);
        currentParse.parse();
        errors = currentParse.getErrors();

        cout << endl;

        // CST
        if (errors > 0)
        {
            log("INFO", "CST for Program #" + to_string(i + 1) + " skipped due to Parse error(s)");
            continue;
        }

        // SEMANATIC ANALYSIS
        // CODE GEN
    }
}

// Logging function for overall Compiler
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
        cout << "Compiler - ";

        cout << message << endl;
    }
}

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
    
    if (input.back() != delimiter || insideComment)
    {
        segments.emplace_back(str);
    }

    return segments;
}