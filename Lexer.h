#ifndef LEXER_H
#define LEXER_H

#include <unordered_map>

// Globals
int LINE = 1;
int COLUMN = 1;
bool VERBOSE = true;

// The Lexer Class
class Lexer 
{
    public:
        // Default constructor for the Lexer class
        Lexer(const int progNum, const string inputCode, const char del)
        {
            this->programNumber = progNum;
            this->program = inputCode;
            this->delimiter = del;
        }

        // Tokenize the program to be used by the Parser
        pair<vector<Token>, int> tokenize()
        {
            vector<Token> tokens;
            smatch match;

            // Print starting INFO for this program
            log("INFO", "Lexing Program #" + to_string(programNumber));

            int size = program.size();
            while (currentPosition < size)
            {
                // Beginning of program snippet represents what's trying to be matched with RegEx
                string programSnippet = program.substr(currentPosition);
                vector< pair <string, string> > matches;

                // Ignores all comments and whitespace
                if (regex_search(programSnippet, match, commentREGEX) || (!inQuotes && regex_search(programSnippet, match, spaceREGEX)))
                {
                    // If comment spanned multiple lines, adjust the LINE value
                    string matched = match.str(0);
                    int newLines = count(matched.begin(), matched.end(), '\n');
                    if (newLines > 0)
                    {
                        LINE += newLines;
                    }
                    
                    currentPosition += match.length(0);
                    COLUMN += match.length(0);

                    continue;
                }
                // Detect if comment opens and doesn't close
                else if (regex_search(programSnippet, match, commentBeginREGEX))
                {
                    log("WARNING", "Unterminated comment", LINE, COLUMN);
                    warningCount++;
                    currentPosition += match.length(0);
                    COLUMN += match.length(0);
                    openComment = true;
                    continue;
                }
                // Detect if comment closed but it never opened
                else if (regex_search(programSnippet, match, commentEndREGEX))
                {
                    log("ERROR", "Unpaired */", LINE, COLUMN);
                    errorCount++;
                    currentPosition += match.length(0);
                    COLUMN += match.length(0);
                    continue;
                }

                // Open comments leave no tokens, so skip iteration
                if (openComment)
                {
                    currentPosition++;
                    COLUMN++;
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

                        // For unterminated strings
                        lastQuoteRow = LINE;
                        lastQuoteCol = COLUMN;
                    }
                    // If code moves on to a new line, adjust line row/column values
                    else if (match.str(0) == "\n" || match.str(0) == "\r" || match.str(0) == "\r\n")
                    {
                        // If still in quotes at the end of a line, there is an unterminated string
                        if (inQuotes)
                        {
                            log("ERROR", "Unterminated string", lastQuoteRow, lastQuoteCol);
                            errorCount++;
                            inQuotes = false;
                        }

                        COLUMN = 1;
                        LINE++;
                        currentPosition += match.length(0);
                        
                        continue;
                    }
                    // Symbols cannot appear in quotes, so throw error
                    else if (inQuotes)
                    {
                        log("ERROR", "Unrecognized Token [ " + string(1, programSnippet[0]) + " ]", LINE, COLUMN);
                        errorCount++;
                        currentPosition++;
                        COLUMN++;
                        continue;
                    }

                    matches.emplace_back(symToName(match.str(0)), match.str(0));
                }
                // Detects digits
                if (!inQuotes && regex_search(programSnippet, match, digitREGEX))
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
                    log("DEBUG", longestMatch.first + " [ " + longestMatch.second + " ] found", LINE, COLUMN);

                    // Create token
                    tokens.emplace_back(Token(longestMatch.first, longestMatch.second, LINE, COLUMN));

                    // Move to next section of the program
                    currentPosition += longestMatch.second.length();
                    COLUMN += longestMatch.second.length();
                    matches.clear();
                }
                // If there were no matches, there was an unrecognized token
                else
                {
                    log("ERROR", "Unrecognized Token [ " + string(1, programSnippet[0]) + " ]", LINE, COLUMN);
                    errorCount++;
                    COLUMN++;
                    currentPosition++;
                }
            }

            // If still in quotes at the end of the program, there is an unterminated string
            if (inQuotes)
            {
                log("ERROR", "Unterminated string", lastQuoteRow, lastQuoteCol);
                errorCount++;
            }

            // Test if last character is the delimiter for warning error
            if (program.back() != delimiter || openComment)
            {
                log("WARNING", "The final program didn't end with a '$', should be", LINE, COLUMN);
                warningCount++;
            }
            
            // Print ending INFO for this program
            log("INFO", "Lex completed with " + to_string(errorCount) + " error(s) and " + to_string(warningCount) + " warning(s)");

            // Returns tokens for this program
            return make_pair(tokens, errorCount);
        }

    private:
        // Holds all of the code in the file
        int programNumber = -1;
        string program;
        char delimiter;

        int currentPosition = 0;
        bool inQuotes = false;
        bool openComment = false;

        // For total error/warning count
        int errorCount = 0;
        int warningCount = 0;
        
        // Determines positions of unterminated strings
        int lastQuoteRow = 0;
        int lastQuoteCol = 0;

        // Regular expressions for this entire grammar
        const regex commentREGEX = regex(R"(^\/\*[\s\S]*?\*\/)");
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
        string symToName(const string symbol)
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

        // Logs message for Lexer
        void log(const string type, const string message, const int row = -1, const int column = -1)
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
                cout << "Lexer - ";

                // If it is of type INFO, print starting or ending message for Lexer 
                if (type == "INFO")
                {
                    cout << message << endl;
                }
                else
                {
                    cout << message << " at (" << row << ":" << column << ")" << endl;
                }
            }
        }
};

#endif