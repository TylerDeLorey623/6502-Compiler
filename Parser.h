#ifndef PARSER_H
#define PARSER_H

using namespace std;

// The Parser Class
class Parser
{
    public:
        // Default constructor for the Parser class
        Parser(const int progNum, const vector<Token> lexTokens)
        {
            this->programNumber = progNum;
            this->tokens = lexTokens; 
        }

        // Validates the tokens
        void validate()
        {
            
        }

    private:
        int programNumber;
        vector<Token> tokens;
};

#endif