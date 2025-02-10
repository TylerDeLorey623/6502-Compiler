#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>

#include "Token.h"
#include "Symbol.h"
#include "Keyword.h"

using namespace std;

class Lexer 
{
    public:
        Lexer(const string inputCode)
        {
            this->program = inputCode;
        }

        // Tokenize the program to be used by the Parser
        vector<Token> tokenize()
        {
            vector<Token> tokens;

            return tokens;
        }

    private:
        // Holds all of the code in the file
        string program;

        // Regular expressions for this entire grammar
        regex keywordREGEX = regex("\\b(print|while|if|int|string|boolean|true|false)\\b");
        regex idREGEX = regex("[a-z]+");
        regex symbolREGEX = regex(R"(\{|\}|"|\(|\)|==|!=|\+|=|\s|\$)");
        regex digitREGEX = regex("[0-9]");
        regex charREGEX = regex("[a-z]");
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
        Lexer* currentLex = new Lexer(programs[i]);
        vector<Token> tokens = currentLex->tokenize();
        // PARSER
        // SEMANATIC ANALYSIS
        // CODE GEN
        delete(currentLex);
    }

    //cout << "DEBUG Lexer - " << savedItem << " [ " <<  savedBuffer << " ] found at (" << savedItemLine << ":" << savedItemColumn << ")" << endl;

    // Close file
    file.close();

}