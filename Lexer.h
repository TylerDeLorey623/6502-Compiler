#ifndef LEXER_H
#define LEXER_H

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
                vector< pair <string, string> > matches;

                // Ignores all comments and whitespace
                if (regex_search(programSnippet, match, commentREGEX) || (!inQuotes && regex_search(programSnippet, match, spaceREGEX)))
                {
                    currentPosition += match.length(0);
                    continue;
                }

                // Detects keywords
                if (regex_search(programSnippet, match, keywordREGEX))
                {
                    matches.emplace_back("KEYWORD", match.str(0));
                }
                // Detects IDs
                if (regex_search(programSnippet, match, idREGEX))
                {
                    matches.emplace_back("ID", match.str(0));
                }
                // Detects symbols
                if (regex_search(programSnippet, match, symbolREGEX))
                {
                    matches.emplace_back("SYMBOL", match.str(0));
                }
                // Detects digits
                if (regex_search(programSnippet, match, digitREGEX))
                {
                    matches.emplace_back("DIGIT", match.str(0));
                }
                // Detects chars
                if (inQuotes && regex_search(programSnippet, match, charREGEX))
                {
                    matches.emplace_back("CHAR", match.str(0));
                }

                // Gets the longest match
                if (!matches.empty()) {
                    pair<string, string> longestMatch = matches[0];

                    // Find the longest match (with values at the beginning of vector having priority for rule order)
                    for (int i = 0, n = matches.size(); i < n; i++)
                    {
                        string currentValue = matches[i].first;
                        string currentWord = matches[i].second;
                        if (currentWord.length() > longestMatch.second.length())
                        {
                            longestMatch.first = currentValue;
                            longestMatch.second = currentWord;
                        }
                    }

                    cout << longestMatch.first << " " << longestMatch.second << endl;
                    currentPosition += longestMatch.second.length();
                }
                // If there were no matches, there was an unrecognized token
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

        int errorCount = 0;

        // Regular expressions for this entire grammar
        regex commentREGEX = regex(R"(^\/\*.*?\*\/)");
        regex spaceREGEX = regex(R"(^\s+)");
        regex keywordREGEX = regex(R"(^\b(print|while|if|int|string|boolean|true|false)\b)");
        regex idREGEX = regex(R"(^[a-z])");
        regex symbolREGEX = regex(R"(^(\{|\}|"|\(|\)|==|!=|\+|=|\$))");
        regex digitREGEX = regex(R"(^[0-9])");
        regex charREGEX = regex(R"(^[a-zA-Z])");
};

#endif