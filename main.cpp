#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include "parser.cpp"
#include <sstream>
#include "symbolTable.cpp"

using namespace std;

string getOutputName(string);
bool removeComments(string, stringstream &);
bool tokenize(stringstream &);
bool isDigit(char);
bool isLetter(char);
bool isHexDigit(char);
bool isSingleTerminator(char);
bool isDoubleTerminator(char);
void addToken(string type, string token, int line);

vector<tuple<string, string, int>> tokenList;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "hi" << endl;
        cout << argv[0] << endl;
        cout << "Incorrect number of arguments" << endl;
        cout << "Format: ./main [input file name]" << endl;
        return -1;
    }
    string input = argv[1];
    stringstream commentsRemoved;
    // queue<pair<string, string>> tokenList;

    if (!removeComments(input, commentsRemoved))
    {
        return -1;
    }

    if (!tokenize(commentsRemoved))
    {
        return -1;
    }

    /*tuple<string, string, int> temp;
    for (int i = 0; i < tokenList.size(); i++)
    {
        temp = tokenList.at(i);
        cout << get<0>(temp) << " " << get<1>(temp) << " " << get<2>(temp) << endl;
    }*/

    string outputName = getOutputName(input);
    ofstream output;
    output.open(outputName);
    Parser parser(tokenList);
    if (parser.parse())
    {
        cout << "Successfully parsed" << endl;
    }
    else
    {
        cout << "Parsing unsuccesful" << endl;
        return;
    }
    SymbolTable SymbolTable(parser.getCST());
    if (SymbolTable.formSymbolTable())
    {
        cout << "Symbol table formed" << endl;
        SymbolTable.printTable(output);
        cout << "Output in " << outputName << endl;
    }
    else
    {
        cout << "Symbol table creation unsuccesful" << endl;
    }
    output.close();
    return 0;
}

bool tokenize(stringstream &input)
{
    char nextChar;
    char prevChar = ' ';
    string currToken = "";
    string tokenType = "";
    int state = 0;
    int line = 1;
    while (input.get(nextChar) && state != -1)
    {
        /*cout << "-----" << endl;
        cout << "State: " << state << endl;
        cout << "Current token: " << currToken << endl;
        cout << "Curr: " << nextChar << endl;
        cout << "Prev: " << prevChar << endl;
        cout << "Type: " << tokenType << endl;*/

        if (state == 0) // starting state
        {
            if (nextChar == ' ' || nextChar == '\n')
            {
            }
            else if (isDigit(nextChar))
            {
                state = 3;
                tokenType = "INTEGER";
                currToken = nextChar;
            }
            else if (isLetter(nextChar) || nextChar == '_')
            {
                state = 4;
                tokenType = "IDENTIFIER";
                currToken = nextChar;
            }
            else if (isSingleTerminator(nextChar))
            {
                state = 5;
            }
            else if (isDoubleTerminator(nextChar))
            {
                state = 6;
            }
        }
        else if (state == 1) // single quoted string
        {
            if (nextChar == '\'')
            {
                addToken(tokenType, currToken, line);
                addToken("SINGLE_QUOTE", "\'", line);
                tokenType = "";
                currToken = "";
                state = 0;
            }
            else
            {
                currToken += nextChar;
                if (nextChar == '\\')
                {
                    state = 11;
                }
            }
        }
        else if (state == 11) // single quoted string with escape character
        {
            currToken += nextChar;
            state = 1;
        }
        else if (state == 2) // doubled quoted string
        {
            if (nextChar == '"')
            {
                addToken(tokenType, currToken, line);
                addToken("DOUBLE_QUOTE", "\"", line);
                tokenType = "";
                currToken = "";
                state = 0;
            }
            else
            {
                currToken += nextChar;
                if (nextChar == '\\')
                {
                    state = 21;
                }
            }
        }
        else if (state == 21) // double quoted string with escape character
        {
            currToken += nextChar;
            state = 2;
        }
        else if (state == 3) // integer
        {
            if (isDigit(nextChar))
            {
                currToken += nextChar;
            }
            else if (isLetter(nextChar) || nextChar == '_')
            {
                state = -1;
                cout << "Syntax error on line " << line << ": invalid integer" << endl;
            }
            else if (isSingleTerminator(nextChar))
            {
                state = 5;
            }
            else if (isDoubleTerminator(nextChar))
            {
                state = 6;
            }
            else
            {
                cout << "Unexpected character on line " << line << ": " << nextChar << endl;
            }
        }
        else if (state == 4) // identifier
        {
            if (nextChar == '_' || isDigit(nextChar) || isLetter(nextChar))
            {
                currToken += nextChar;
            }
            else if (isSingleTerminator(nextChar))
            {
                state = 5;
            }
            else if (isDoubleTerminator(nextChar))
            {
                state = 6;
            }
            else
            {
                cout << "Unexpected character on line " << line << ": " << nextChar << endl;
            }
        }
        else if (state == 5) // single character tokens that terminate the previous token
        {
            if (currToken != "")
            {
                // print previous token
                addToken(tokenType, currToken, line);
            }
            // get new token type
            switch (prevChar)
            {
            case ' ':
                state = 0;
                tokenType = "";
                break;
            case '\n':
                state = 0;
                tokenType = "";
                break;
            case '\'':
                tokenType = "SINGLE_QUOTE";
                state = 1;
                break;
            case '"':
                tokenType = "DOUBLE_QUOTE";
                state = 2;
                break;
            case '(':
                tokenType = "L_PAREN";
                break;
            case ')':
                tokenType = "R_PAREN";
                break;
            case '[':
                tokenType = "L_BRACKET";
                break;
            case ']':
                tokenType = "R_BRACKET";
                break;
            case '{':
                tokenType = "L_BRACE";
                break;
            case '}':
                tokenType = "R_BRACE";
                break;
            case ';':
                tokenType = "SEMICOLON";
                break;
            case ',':
                tokenType = "COMMA";
                break;
            case '%':
                tokenType = "MODULO";
                break;
            case '*':
                tokenType = "ASTERISK";
                break;
            case '/':
                tokenType = "DIVIDE";
                break;
            default:
                cout << "Unexpected terminator on line " << line << ": " << prevChar << endl;
            }
            currToken = prevChar;
            if (tokenType != "")
            {
                // output new token if it wasn't a space or \n
                addToken(tokenType, currToken, line);
            }
            currToken = "";
            if (state == 1 || state == 2)
            {
                tokenType = "STRING";
                currToken = nextChar;
            }
            else if (isDigit(nextChar))
            {
                state = 3;
                tokenType = "INTEGER";
                currToken = nextChar;
            }
            else if (isLetter(nextChar) || nextChar == '_')
            {
                state = 4;
                tokenType = "IDENTIFIER";
                currToken = nextChar;
            }
            else if (isSingleTerminator(nextChar))
            {
                state = 5;
            }
            else if (isDoubleTerminator(nextChar))
            {
                state = 6;
            }
        }
        else if (state == 6) // (possibly) multicharacter terminator tokens
        {
            // eg: <=, >=, !=, ==, ||, &&, +(int), -(int)
            string oldType = "";
            string oldToken;
            if (currToken != "")
            {
                // print previous token
                addToken(tokenType, currToken, line);
            }

            switch (prevChar)
            {
            case '<':
                tokenType = "LT";
                break;
            case '>':
                tokenType = "GT";
                break;
            case '!':
                tokenType = "BOOLEAN_NOT";
                break;
            case '+':
                tokenType = "PLUS";
                break;
            case '-':
                tokenType = "MINUS";
                break;
            case '=':
                tokenType = "ASSIGNMENT_OPERATOR";
                break;
            }
            currToken = prevChar;
            if (nextChar == ' ' || nextChar == '\n')
            {
                if (tokenType != "")
                {
                    // print single character terminator tokens that were followed by white space
                    // <, >, !, +, -, or =
                    addToken(tokenType, currToken, line);
                    currToken = "";
                    state = 0;
                }
                else
                {
                    // invalid: & (should be &&) and | (should be ||)
                    state = -1;
                    cout << "Syntax error on line " << line << ": unexpected space following " << prevChar << endl;
                }
            }
            else if (isDigit(nextChar))
            {
                if (prevChar == '+' || prevChar == '-')
                {
                    currToken += nextChar;
                }
                else
                {
                    addToken(tokenType, currToken, line);
                    currToken = nextChar;
                }
                state = 3;
                tokenType = "INTEGER";
            }
            else if (isLetter(nextChar) || nextChar == '_')
            {
                addToken(tokenType, currToken, line);
                state = 4;
                tokenType = "IDENTIFIER";
                currToken = nextChar;
            }
            else if (nextChar == '=')
            {
                if (prevChar == '<' || prevChar == '>' || prevChar == '!')
                {
                    currToken += nextChar;
                    tokenType += "_EQUAL";
                    addToken(tokenType, currToken, line);
                    currToken = "";
                    tokenType = "";
                    state = 0;
                }
                else if (prevChar == '=')
                {
                    addToken("BOOLEAN_EQUAL", "==", line);
                    currToken = "";
                    tokenType = "";
                    state = 0;
                }
                else
                {
                    state = -1;
                    cout << "Syntax error on line " << line << ": unexpected = following " << prevChar << endl;
                }
            }
            else if (nextChar == '&' && prevChar == '&')
            {
                addToken("BOOLEAN_AND", "&&", line);
                currToken = "";
                tokenType = "";
                state = 0;
            }
            else if (nextChar == '|' && prevChar == '|')
            {
                addToken("BOOLEAN_OR", "||", line);
                currToken = "";
                tokenType = "";
                state = 0;
            }
            else if (isSingleTerminator(nextChar))
            {
                addToken(tokenType, currToken, line);
                state = 5;
                currToken = "";
                tokenType = "";
            }
            else
            {
                state = -1;
                cout << "Syntax error on line " << line << "" << endl;
            }
        }

        if (nextChar == '\n')
        {
            line++;
        }

        prevChar = nextChar;
    }

    bool returnVal;

    if (state == 1 || state == 2)
    {
        cout << "Syntax error on line " << line << ": unterminated string" << endl;
        returnVal = false;
    }
    else if (state == -1)
    {
        returnVal = false;
    }
    else
    {
        if (currToken != "")
        {
            addToken(tokenType, currToken, line);
        }
        cout << "Token list created successfully" << endl;
        returnVal = true;
    }

    return returnVal;
}

bool removeComments(string inName, stringstream &output)
{
    ifstream input;
    input.open(inName);

    if (!input)
    {
        cout << "File \"" << inName << "\" failed to open, please check file name" << endl;
        input.close();
        return false;
    }

    char nextChar;
    int state = 0;
    int line = 1;
    int commentStart = -1;
    bool printChar = false;

    while (input.get(nextChar) && state != -1)
    {
        /*cout << "state: " << state << endl;
        cout << "current: " << nextChar << endl;*/
        printChar = false;
        if (state == 0) // waiting for start of comment
        {
            if (nextChar == '/')
            {
                state = 1;
                commentStart = line;
            }
            else if (nextChar == '*')
            {
                state = 5;
                commentStart = line;
            }
            else if (nextChar == '"')
            {
                state = 6;
                printChar = true;
            }
            else
            {
                printChar = true;
            }
        }
        else if (state == 1) // checking if single line comment or block comment
        {
            if (nextChar == '/')
            {
                state = 2;
            }
            else if (nextChar == '*')
            {
                state = 3;
            }
            else
            {
                state = 0; // false alarm, '/' was not a comment
                output << '/';
                printChar = true;
            }
        }
        else if (state == 2) // single-line comment
        {
            if (nextChar == '\n')
            {
                state = 0;
            }
        }
        else if (state == 3) // block comment
        {
            if (nextChar == '*')
            {
                state = 4;
            }
        }
        else if (state == 4) // check for end of block comment
        {
            if (nextChar == '/')
            {
                state = 0;
            }
            else if (nextChar != '*')
            {
                state = 3;
            }
        }
        else if (state == 5) // check for dangling '*/'
        {
            if (nextChar == '/')
            {
                state = -1;
            }
            else
            {
                state = 0; // false alarm
                output << '*';
                printChar = true;
            }
        }
        else if (state == 6) // inside a string
        {
            if (nextChar == '"')
            {
                state = 0;
            }
            printChar = true;
        }
        if (nextChar == '\n')
        {
            line++;
            printChar = true;
        }
        if (printChar)
        {
            output << nextChar;
        }
    }

    input.close();

    if (state == 0 || state == 2 || state == 6)
    {
        cout << "Comments succesfully removed" << endl;
        return true;
    }
    else
    {
        cout << "ERROR: Program contains C-style, unterminated comment on line " << commentStart << endl;
        return false;
    }
}

bool isLetter(char c)
{
    int ascii = (int)c;
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

bool isHexDigit(char c)
{
    return isDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool isSingleTerminator(char c)
{
    return c == '{' || c == '}' || c == '(' || c == ')' || c == '[' || c == ']' || c == ';' || c == ',' || c == '.' || c == '^' || c == '%' || c == '\'' || c == '"' || c == '*' || c == '/' || c == ' ' || c == '\n';
}

bool isDoubleTerminator(char c)
{
    return c == '=' || c == '|' || c == '&' || c == '+' || c == '-' || c == '!' || c == '<' || c == '>';
}

string getOutputName(string inputName)
{
    int end = inputName.find('.');
    string outputName = inputName.substr(0, end);
    return outputName + "_symbol_table.txt";
}

void addToken(string type, string token, int line)
{
    // cout << "Token finished! " << token << " " << endl;
    tuple<string, string, int> newToken = make_tuple(type, token, line);
    tokenList.push_back(newToken);
    return;
}