#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>

#include "Token.h"

using namespace std;

class Lexer 
{
    public:
        // Default constructor for the Lexer class
        Lexer(const string inputCode)
        {
            this->program = inputCode;
        }

        // Tokenize the program to be used by the Parser
        pair<vector<Token>, int> tokenize()
        {
            vector<Token> tokens;
            smatch match;

            int size = program.size();
            while (currentPosition < size)
            {
                string programSnippet = program.substr(currentPosition);

                // Ignores all comments
                if (regex_search(programSnippet, match, commentREGEX))
                {
                    currentPosition += match.length(0);
                    continue;
                }
                // Ignores whitespace
                else if (!inQuotes && regex_search(programSnippet, match, spaceREGEX))
                {
                    currentPosition += match.length(0);
                    continue;
                }

                // Detects keywords
                if (regex_search(programSnippet, match, keywordREGEX))
                {
                    cout << "KEYWORD " << match.str(0) << endl;
                    currentPosition += match.length(0);
                }
                // Detects IDs
                else if (regex_search(programSnippet, match, idREGEX))
                {
                    cout << "ID " << match.str(0) << endl;
                    currentPosition += match.length(0);
                    
                    requireID = false;
                }
                // Detects symbols
                else if (regex_search(programSnippet, match, symbolREGEX))
                {
                    cout << "SYMBOL " << match.str(0) << endl;
                    currentPosition += match.length(0);
                }
                // Detects digits
                else if (regex_search(programSnippet, match, digitREGEX))
                {
                    cout << "DIGIT " << match.str(0) << endl;
                    currentPosition += match.length(0);
                }
                // Detects chars
                else if (inQuotes && regex_search(programSnippet, match, charREGEX))
                {
                    cout << "CHAR " << match.str(0) << endl;
                    currentPosition += match.length(0);
                }
                else
                {
                    cout << "Unrecognized Token: " << programSnippet[0] << endl;
                    errorCount++;
                    break;
                }

            }


            return make_pair(tokens, errorCount);
        }

    private:
        // Holds all of the code in the file
        string program;

        int currentPosition = 0;
        bool inQuotes = false;

        string storedKeyword;
        bool requireID = false;

        int errorCount = 0;

        // Regular expressions for this entire grammar
        regex commentREGEX = regex(R"(^\/\*.*?\*\/)");
        regex spaceREGEX = regex(R"(^\s+)");
        regex keywordREGEX = regex(R"(^\b(print|while|if|int|string|boolean|true|false)\b)");
        regex idREGEX = regex(R"(^[a-z]+)");
        regex symbolREGEX = regex(R"(^(\{|\}|"|\(|\)|==|!=|\+|=|\$))");
        regex digitREGEX = regex(R"(^[0-9])");
        regex charREGEX = regex(R"(^[a-zA-Z])");
};

// Splits up the programs by the delimiter
vector<string> splitPrograms(const string input, const char delimiter)
{
    vector<string> segments;
    stringstream stream(input);
    string str;

    while (getline(stream, str, delimiter))
    {
        segments.push_back(str);
    }

    // Test if last character is the delimiter for warning error
    if (input.back() != delimiter)
    {
        cout << "WARNING: The final program does not end with a '" << delimiter << "'." << endl;
    }

    return segments;
}

int main() 
{
    // File IO
    ifstream file("code.txt");
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