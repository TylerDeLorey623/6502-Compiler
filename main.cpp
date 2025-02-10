#include <iostream>
#include <fstream>
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
            this->code = inputCode;
        }

        vector<Token> tokenize()
        {

        }

    private:
        // Holds all of the code in the file
        string code;

        // Regular expressions for this entire grammar
        regex keywordREGEX = regex("\\b(print|while|if|int|string|boolean|true|false)\\b");
        regex idREGEX = regex("[a-z]+");
        regex symbolREGEX = regex(R"(\{|\}|"|\(|\)|==|!=|\+|=|\s|\$)");
        regex digitREGEX = regex("[0-9]");
        regex charREGEX = regex("[a-z]");
};

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