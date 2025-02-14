#ifndef LEXER_H
#define LEXER_H

#include <unordered_map>

// Globals
int LINEROW = 1;
int LINECOLUMN = 1;

// The Lexer Class
class Lexer 
{
    public:
        // Default constructor for the Lexer class
        Lexer(const string inputCode, const char del)
        {
            this->program = inputCode;
            this->delimiter = del;
        }

        // Tokenize the program to be used by the Parser
        pair<vector<Token>, pair<int, int> > tokenize()
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
                    LINECOLUMN += match.length(0);
                    continue;
                }
                // Detect if comment opens and doesn't close
                else if (regex_search(programSnippet, match, commentBeginREGEX))
                {
                    cout << "ERROR Lexer - Unclosed Comment starting at (" << LINEROW << ":" << LINECOLUMN << ")" << endl;
                    errorCount++;
                    currentPosition += match.length(0);
                    LINECOLUMN += match.length(0);
                    openComment = true;
                    continue;
                }
                // Detect if comment closed but it never opened
                else if (regex_search(programSnippet, match, commentEndREGEX))
                {
                    cout << "ERROR Lexer - Unpaired */ token at (" << LINEROW << ":" << LINECOLUMN << ")" << endl;
                    errorCount++;
                    currentPosition += match.length(0);
                    LINECOLUMN += match.length(0);
                    continue;
                }

                // Open comments leave no tokens, so skip iteration
                if (openComment)
                {
                    currentPosition++;
                    LINECOLUMN++;
                    continue;
                }

                // Detects keywords
                if (!inQuotes && regex_search(programSnippet, match, keywordREGEX))
                {
                    matches.emplace_back(symToName(match.str(0)), match.str(0));
                }
                // Detects IDs
                if (!inQuotes && regex_search(programSnippet, match, idREGEX))
                {
                    matches.emplace_back("ID", match.str(0));
                }
                // Detects symbols
                if (regex_search(programSnippet, match, symbolREGEX))
                {
                    // Flip boolean value to detect characters if quotes appear
                    if (match.str(0) == "\"")
                    {
                        if (inQuotes)
                        {
                            inQuotes = false;
                        }
                        else
                        {
                            inQuotes = true;
                        }

                        // For unclosed strings
                        lastQuoteRow = LINEROW;
                        lastQuoteCol = LINECOLUMN;
                    }
                    // If code moves on to a new line, adjust line row/column values
                    else if (match.str(0) == "\n" || match.str(0) == "\r" || match.str(0) == "\r\n")
                    {
                        // If still in quotes at the end of a line, there is an unclosed string
                        if (inQuotes)
                        {
                            cout << "ERROR Lexer - Unclosed string starting at (" << lastQuoteRow << ":" << lastQuoteCol << ")" << endl;
                            errorCount++;
                            inQuotes = false;
                        }

                        LINECOLUMN = 1;
                        LINEROW++;
                        currentPosition += match.length(0);
                        
                        continue;
                    }
                    // Symbols cannot appear in quotes, so throw error
                    else if (inQuotes)
                    {
                        cout << "ERROR Lexer - Unrecognized Token [ " << programSnippet[0] << " ] at (" << LINEROW << ":" << LINECOLUMN << ")" << endl;
                        errorCount++;
                        currentPosition++;
                        LINECOLUMN++;
                        continue;
                    }

                    matches.emplace_back(symToName(match.str(0)), match.str(0));
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

                    // Print results
                    cout << "DEBUG Lexer - " << longestMatch.first << " [ " << longestMatch.second << " ] found at (" << LINEROW << ":" << LINECOLUMN << ")" << endl;

                    // Create token
                    tokens.emplace_back(Token(longestMatch.first, longestMatch.second, LINEROW, LINECOLUMN));

                    // Move to next section of the program
                    currentPosition += longestMatch.second.length();
                    LINECOLUMN += longestMatch.second.length();
                    matches.clear();
                }
                // If there were no matches, there was an unrecognized token
                else
                {
                    cout << "ERROR Lexer - Unrecognized Token [ " << programSnippet[0] << " ] at (" << LINEROW << ":" << LINECOLUMN << ")" << endl;
                    errorCount++;
                    LINECOLUMN++;
                    currentPosition++;
                }
            }

            // If still in quotes at the end of the program, there is an unclosed string
            if (inQuotes)
            {
                cout << "ERROR Lexer - Error: Unclosed string starting at (" << lastQuoteRow << ":" << lastQuoteCol << ")" << endl;
                errorCount++;
            }

            // Test if last character is the delimiter for warning error
            if (program.back() != delimiter || openComment)
            {
                cout << "WARNING Lexer: The final program didn't end with a '$'" << endl;
                warningCount++;
            }

            return make_pair(tokens, make_pair(errorCount, warningCount));
        }

    private:
        // Holds all of the code in the file
        string program;
        char delimiter;

        int currentPosition = 0;
        bool inQuotes = false;
        bool openComment = false;

        // For total error/warning count
        int errorCount = 0;
        int warningCount = 0;
        
        // Determines positions of unclosed strings
        int lastQuoteRow = 0;
        int lastQuoteCol = 0;

        // Regular expressions for this entire grammar
        const regex commentREGEX = regex(R"(^\/\*.*?\*\/)");
        const regex commentBeginREGEX = regex(R"(^\/\*)");
        const regex commentEndREGEX = regex(R"(^\*\/)");

        const regex spaceREGEX = regex(R"(^[ \t])");
        const regex keywordREGEX = regex("^(print|while|if|int|string|boolean|true|false)");
        const regex idREGEX = regex(R"(^[a-z])");
        const regex symbolREGEX = regex(R"(^(\{|\}|"|\(|\)|==|!=|\+|=|\$|\r?\n))");
        const regex digitREGEX = regex(R"(^[0-9])");
        const regex charREGEX = regex(R"(^[a-z|\s])");

        // Map which corresponds the symbols/keywords to words used for the debugger and tokenization
        unordered_map<string, string> symbolMap = 
        {
            // Keywords
            {"print", "PRINT_STATEMENT"},
            {"while", "WHILE_STATEMENT"},
            {"if", "IF_STATEMENT"},
            {"int", "I_VARTYPE"},
            {"string", "S_VARTYPE"},
            {"boolean", "B_VARTYPE"},
            {"true", "BOOL_VAL"},
            {"false", "BOOL_VAL"},

            // Symbols
            {"{", "OPEN_CURLY"},
            {"}", "CLOSE_CURLY"},
            {"\"", "QUOTE"},
            {"(", "OPEN_PARENTHESIS"},
            {")", "CLOSE_PARENTHESIS"},
            {"==", "EQUALITY_OP"},
            {"!=", "INEQUALITY_OP"},
            {"+", "ADDITION_OP"},
            {"=", "ASSIGNMENT_OP"},
            {"$", "EOP"}
        };

        // Converts any symbol or keyword to a name
        string symToName(string symbol)
        {
            string printVal;

            // Search for the symbol and return its name
            if (symbolMap.find(symbol) != symbolMap.end()) 
            {
                printVal = symbolMap[symbol];
            }
            else 
            {
                printVal = "UNKNOWN"; 
            }

            return printVal;
        }
};

#endif