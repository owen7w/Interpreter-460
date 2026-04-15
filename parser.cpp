#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#ifndef PARSER
#define PARSER

using namespace std;

class TreeNode
{
public:
    TreeNode(string n, int l) : left(nullptr), right(nullptr), data(n), line(l) {}
    ~TreeNode()
    {
        leftChild(nullptr);
        rightSibling(nullptr);
    }

    TreeNode *leftChild() { return left; }
    TreeNode *rightSibling() { return right; }

    void leftChild(TreeNode *newLeft) { left = newLeft; }
    void rightSibling(TreeNode *newRight) { right = newRight; }

    string token() { return data; }
    int getLine() { return line; }

private:
    TreeNode *left, *right;
    string data;
    int line;
};

class CST
{
public:
    CST() : head(nullptr), tail(nullptr), size(0) {}
    ~CST()
    {
        destroy(head);
    }
    void addChild(string token, int line)
    {
        size++;
        if (head == nullptr)
        {
            head = new TreeNode(token, line);
            tail = head;
        }
        else
        {
            tail->leftChild(new TreeNode(token, line));
            tail = tail->leftChild();
        }
    };
    void addSibling(string token, int line)
    {
        size++;
        if (head == nullptr)
        {
            head = new TreeNode(token, line);
            tail = head;
        }
        else
        {
            tail->rightSibling(new TreeNode(token, line));
            tail = tail->rightSibling();
        }
    };
    int getSize()
    {
        return size;
    }
    TreeNode *getHead()
    {
        return head;
    }
    void printTree(ofstream &output)
    {
        TreeNode *temp = head;
        while (temp != NULL)
        {
            output << temp->token() << " ";
            if (temp->rightSibling() == NULL)
            {
                output << endl;
                temp = temp->leftChild();
            }
            else
            {
                temp = temp->rightSibling();
            }
        }
    }

private:
    TreeNode *head, *tail;
    int size;
    void destroy(TreeNode *root)
    {
        if (root == nullptr)
        {
            return;
        }
        destroy(root->leftChild());
        destroy(root->rightSibling());
        delete root;
    }
};

class Parser
{
public:
    vector<tuple<string, string, int>> &tokenList;

    Parser(vector<tuple<string, string, int>> &TL) : tokenList(TL), index(0), errorFound(false), log(false), cst(new CST()) {}

    bool parse()
    {
        return isProgram();
    }

    CST *getCST()
    {
        return cst;
    }

    void printCST(ofstream &output)
    {
        cst->printTree(output);
    }

private:
    int index;
    CST *cst;
    bool errorFound;
    bool log;

    void addStatement()
    {
        bool stop = false;
        if (nextLine() > 33)
        {
            stop = true;
        }
        int prevIdx = cst->getSize();
        cst->addChild(get<1>(tokenList.at(prevIdx)), get<2>(tokenList.at(prevIdx)));
        prevIdx++;
        while (prevIdx <= index)
        {
            cst->addSibling(get<1>(tokenList.at(prevIdx)), get<2>(tokenList.at(prevIdx)));
            prevIdx++;
        }
    }
    string nextType()
    {
        return get<0>(tokenList.at(index));
    }
    string nextToken()
    {
        return get<1>(tokenList.at(index));
    }
    int nextLine()
    {
        return get<2>(tokenList.at(index));
    }
    bool syntaxError(string error)
    {
        if (!errorFound) // dont print extra errors
        {
            cout << endl;
            cout << "Syntax error on line " << nextLine() << ": ";
            cout << error << endl;
        }
        if (log)
        {
            cout << "Current token: " << nextToken() << " <" << nextType() << ">" << endl;
        }
        errorFound = true;
        return false;
    }
    bool syntaxErrorUnexpected(string token)
    {
        cout << endl;
        cout << "Syntax error on line " << nextLine() << ", following '";
        cout << get<1>(tokenList.at(index - 1)) << "' : ";
        cout << "unexpected token '" << nextToken() << "', expected '" << token << "'." << endl;
        errorFound = true;
        return false;
    }
    void printLog(string context)
    {
        cout << "Checking for " << context << "." << endl;
        cout << "Current token: " << nextToken() << endl;
    }
    bool isEnd()
    {
        if (log)
        {
            cout << "checking for end " << index << " >= " << tokenList.size() << "?" << endl;
        }
        return index >= tokenList.size();
    }
    bool isProgram()
    {
        if (log)
        {
            printLog("program");
        }
        // program must contain a procedure main, this searches for it
        int prevIdx = index; // if possible descent fails, return to original point in token list
        if (isMainProcedure())
        {
            return isEnd() || isProgramTail() ? true : syntaxError("invalid program tail following procedure main.");
        }
        else
        {
            if (errorFound)
            {
                return false;
            }
            index = prevIdx;
            if (isProcedureDeclaration())
            {
                return isProgram() ? true : syntaxError("invalid program. No main?");
            }
            else
            {
                if (errorFound)
                {
                    return false;
                }
                index = prevIdx;
                if (isFunctionDeclaration())
                {
                    return isProgram() ? true : syntaxError("invalid program. No main?");
                }
                else
                {
                    if (errorFound)
                    {
                        return false;
                    }
                    index = prevIdx;
                    if (isDeclarationStatement())
                    {
                        return isProgram() ? true : syntaxError("invalid program. No main?");
                    }
                    else
                    {
                        return syntaxError("invalid program structure.");
                    }
                }
            }
        }
    }

    bool isProgramTail()
    {
        if (log)
        {
            printLog("program tail");
        }
        // the rest of the program (should not contain main procedure)
        int prevIdx = index; // if possible descent fails, return to original point in token list
        if (isProcedureDeclaration())
        {
            return isEnd() || isProgramTail() ? true : syntaxError("invalid program tail.");
        }
        else
        {
            if (errorFound)
            {
                return false;
            }
            index = prevIdx;
            if (isFunctionDeclaration())
            {
                return isEnd() || isProgramTail() ? true : syntaxError("invalid program structure.");
            }
            else
            {
                if (errorFound)
                {
                    return false;
                }
                index = prevIdx;
                if (isDeclarationStatement())
                {
                    return isEnd() || isProgramTail() ? true : syntaxError("invalid program structure.");
                }
                else
                {
                    return syntaxError("invalid program structure.");
                }
            }
        }
    }

    bool isMainProcedure()
    {
        if (log)
        {
            printLog("main procedure");
        }
        if (nextToken() == "procedure")
        {
            index++;
            if (nextToken() == "main")
            {
                index++;
                if (nextType() == "L_PAREN")
                {
                    index++;
                    if (nextToken() == "void")
                    {
                        index++;
                        if (nextType() == "R_PAREN")
                        {
                            addStatement();
                            index++;
                            return isBlockStatement() ? true : syntaxError("invalid block statement following procedure main.");
                        }
                        else
                        {
                            return syntaxErrorUnexpected(")");
                        }
                    }
                    else
                    {
                        return syntaxError("invalid parameters for procedure main. Only void is allowed.");
                    }
                }
                else
                {
                    return syntaxErrorUnexpected("(");
                }
            }
            else // not main procedure, no error
            {
                return false;
            }
        }
        else // not a procedure, no error
        {
            return false;
        }
    }

    bool isProcedureDeclaration()
    {
        if (log)
        {
            printLog("procedure declaration");
        }
        if (nextToken() == "procedure")
        {
            index++;
            if (isValidIdentifier())
            {
                index++;
                if (nextType() == "L_PAREN")
                {
                    index++;
                    if (nextToken() == "void")
                    {
                        index++;
                        if (nextType() == "R_PAREN")
                        {
                            addStatement();
                            index++;
                            return isBlockStatement() ? true : syntaxError("invalid block statement following procedure.");
                        }
                        else
                        {
                            return syntaxErrorUnexpected(")");
                        }
                    }
                    else
                    {
                        if (isParameterList())
                        {
                            if (nextType() == "R_PAREN")
                            {
                                addStatement();
                                index++;
                                return isBlockStatement() ? true : syntaxError("invalid block statement following procedure.");
                            }
                            else
                            {
                                return syntaxErrorUnexpected(")");
                            }
                        }
                        else
                        {
                            return syntaxError("invalid parameter list.");
                        }
                    }
                }
                else
                {
                    return syntaxErrorUnexpected("(");
                }
            }
            else
            {
                return syntaxError("invalid procedure identifier name.");
            }
        }
        else // not procedure, no error
        {
            return false;
        }
    }
    bool isFunctionDeclaration()
    {
        if (log)
        {
            printLog("function declaration");
        }
        if (nextToken() == "function")
        {
            index++;
            if (isValidDatatype())
            {
                index++;
                if (isValidIdentifier())
                {
                    index++;
                    if (nextType() == "L_PAREN")
                    {
                        index++;
                        if (nextToken() == "void")
                        {
                            index++;
                            if (nextType() == "R_PAREN")
                            {
                                addStatement();
                                index++;
                                return isBlockStatement() ? true : syntaxError("invalid block statement following procedure.");
                            }
                            else
                            {
                                return syntaxErrorUnexpected(")");
                            }
                        }
                        else
                        {
                            if (isParameterList())
                            {
                                if (nextType() == "R_PAREN")
                                {
                                    addStatement();
                                    index++;
                                    return isBlockStatement() ? true : syntaxError("invalid block statement following procedure.");
                                }
                                else
                                {
                                    return syntaxErrorUnexpected(")");
                                }
                            }
                            else
                            {
                                return syntaxError("invalid parameter list.");
                            }
                        }
                    }
                    else
                    {
                        return syntaxErrorUnexpected("(");
                    }
                }
                else
                {
                    return syntaxError("invalid function name '" + nextToken() + "'.");
                }
            }
            else
            {
                return syntaxError("invalid return datatype.");
            }
        }
        else // not function, no error
        {
            return false;
        }
    }

    bool isDeclarationStatement()
    {
        if (log)
        {
            printLog("declaration statement");
        }
        if (isValidDatatype())
        {
            index++;
            if (isIdentifierOrIdentifierArrayList())
            {
                if (nextType() == "SEMICOLON")
                {
                    addStatement();
                    index++;
                    return true;
                }
                else
                {
                    return syntaxErrorUnexpected(";");
                }
            }
            else
            {
                return syntaxError("invalid variable name '" + nextToken() + "'.");
            }
        }
        else // not declaration statement, no error
        {
            // return syntaxError("invalid variable datatype.");
            return false;
        }
    }

    bool isBlockStatement()
    {
        if (log)
        {
            printLog("block statement");
        }
        // form: { <COMPOUND_STATEMENT> }
        if (nextType() == "L_BRACE")
        {
            addStatement();
            index++;
            if (isCompoundStatement())
            {
                if (nextType() == "R_BRACE")
                {
                    addStatement();
                    index++;
                    return true;
                }
                else
                {
                    return syntaxError("improperly closed block statement. No '}'.");
                }
            }
            else
            {
                if (nextType() == "R_BRACE")
                {
                    addStatement();
                    index++;
                    return true;
                }
                else
                {
                    return syntaxError("invalid block statement. Doesn't contain valid compound statement.");
                }
            }
        }
        else
        {
            // not a block statement
            return false;
        }
    }

    bool isValidIdentifier()
    {
        if (log)
        {
            printLog("identifier");
        }
        if (nextType() == "IDENTIFIER")
        {
            // check for reserved words
            vector<string> reservedWords = {"function", "procedure", "return", "main", "for", "while", "if", "else", "getchar", "printf", "sizeof", "true", "false", "void"};
            string curr = nextToken();
            bool isReserved = false;
            for (int i = 0; i < reservedWords.size(); i++)
            {
                if (curr == reservedWords.at(i))
                {
                    isReserved = true;
                    break;
                }
            }
            return (!isReserved && !isValidDatatype());
        }
        else
        {
            // not an identifier
            return false;
        }
    }

    bool isParameterList()
    {
        if (log)
        {
            printLog("parameter list");
        }
        if (isValidDatatype())
        {
            index++;
            if (isIdentifierOrIdentifierArrayList())
            {
                if (nextType() == "R_PAREN") // peek ahead to see if list ended
                {
                    return true;
                }
                else
                {
                    if (nextType() == "COMMA")
                    {
                        index++;
                        return isParameterList();
                    }
                    else
                    {
                        return syntaxError("missing comma in parameter list.");
                    }
                }
            }
            else
            {
                return syntaxError("invalid variable '" + nextToken() + "'.");
            }
        }
        else
        {
            // not a parameter list. Should this return a syntax error?
            // return syntaxError("invalid paramter list datatype.");
            return false;
        }
    }

    bool isIdentifierOrIdentifierArrayList()
    {
        if (log)
        {
            printLog("identifier list");
        }
        if (isVariableOrVariableArray())
        {
            if (nextType() == "COMMA")
            {
                int prevIdx = index;
                index++;
                if (isValidDatatype())
                {
                    // list actually ended
                    // ex: int a, b, c, char
                    // should stop at c since next would hit char
                    index = prevIdx;
                    return true;
                }
                else
                {
                    return isIdentifierOrIdentifierArrayList() ? true : syntaxError("identifier list started but improperly continued.");
                }
            }
            else
            {
                return true;
            }
        }
        else
        {
            // not an identifier list, should this print error?
            // return syntaxError("invalid identifier.");
            return false;
        }
    }
    bool isValidDatatype()
    {
        if (log)
        {
            printLog("valid datatype");
        }
        if (nextType() == "IDENTIFIER")
        {
            // check for valid datatype
            // assumes no user defined datatypes
            vector<string> datatypes = {"char", "bool", "int"};
            string curr = nextToken();
            bool isValid = false;
            for (int i = 0; i < datatypes.size(); i++)
            {
                if (curr == datatypes.at(i))
                {
                    isValid = true;
                    break;
                }
            }
            return isValid;
        }
        else
        {
            // not a valid datatype, not an identifier
            return false;
        }
    }
    bool isCompoundStatement()
    {
        if (log)
        {
            printLog("compound statement");
        }
        if (isStatement())
        {
            // addStatement();
            int prevIdx = index; // may need to backtrack
            if (isEnd() || isCompoundStatement())
            {
                return true;
            }
            else
            {
                if (errorFound)
                {
                    return false;
                }
                // previous statement was the last statement, backtrack
                index = prevIdx;
                return true;
            }
        }
        else
        {
            return false;
        }
    }
    bool isStatement()
    {
        if (log)
        {
            printLog("statement");
        }
        int prevIdx = index;
        if (isDeclarationStatement())
        {
            return true;
        }
        else
        {
            if (errorFound)
            {
                return false;
            }
            index = prevIdx;
            if (isAssignmentStatement())
            {
                return true;
            }
            else
            {
                if (errorFound)
                {
                    return false;
                }
                index = prevIdx;
                if (isIterationStatement())
                {
                    return true;
                }
                else
                {
                    if (errorFound)
                    {
                        return false;
                    }
                    index = prevIdx;
                    if (isSelectionStatement())
                    {
                        return true;
                    }
                    else
                    {
                        if (errorFound)
                        {
                            return false;
                        }
                        index = prevIdx;
                        if (isPrintfStatement())
                        {
                            return true;
                        }
                        else
                        {
                            if (errorFound)
                            {
                                return false;
                            }
                            index = prevIdx;
                            if (isReturnStatement())
                            {
                                return true;
                            }
                            else
                            {
                                return isUserDefinedProcedureCallStatement();
                            }
                        }
                    }
                }
            }
        }
    }
    bool isAssignmentStatement()
    {
        // <IDENTIFIER> = ____
        // <IDENTIFIER> [<NUMERICAL_EXPRESSION>] = ____
        // = <EXPRESSION>; OR = <STRING>;
        if (log)
        {
            printLog("assignment statement");
        }

        if (isVariableOrVariableArray())
        {
            if (nextType() == "ASSIGNMENT_OPERATOR")
            {
                index++;
                int prevIdx = index;
                if (isExpression())
                {
                    if (nextType() == "SEMICOLON")
                    {
                        addStatement();
                        index++;
                        return true;
                    }
                    else
                    {
                        return syntaxErrorUnexpected(";");
                    }
                }
                else
                {
                    if (errorFound)
                    {
                        return false;
                    }
                    index = prevIdx;
                    if (isString())
                    {
                        if (nextType() == "SEMICOLON")
                        {
                            addStatement();
                            index++;
                            return true;
                        }
                        else
                        {
                            return syntaxErrorUnexpected(";");
                        }
                    }
                    else
                    {
                        return syntaxError("invalid right hand side of assignment operator.");
                    }
                }
            }
            else
            {
                // not an assignment statement, other variable
                return false;
            }
        }
        else
        {
            // return syntaxError("invalid left hand side of assignment operator, '" + nextToken() + "'.");
            return false;
        }
    }
    bool isIterationStatement()
    {
        if (log)
        {
            printLog("iteration statement");
        }
        // either start of for loop or start of while loop
        // followed by either <STATEMENT> or <BLOCK_STATEMENT>

        bool validStart = false;
        // check for correct start
        if (nextToken() == "for")
        {
            index++;
            if (nextType() == "L_PAREN")
            {
                index++;
                if (isInitializationExpression())
                {
                    if (nextType() == "SEMICOLON")
                    {
                        index++;
                        if (isBooleanExpression())
                        {
                            if (nextType() == "SEMICOLON")
                            {
                                index++;
                                if (isIterationAssignment())
                                {
                                    if (nextType() == "R_PAREN")
                                    {
                                        addStatement();
                                        index++;
                                        validStart = true;
                                    }
                                    else
                                    {
                                        return syntaxErrorUnexpected(")");
                                    }
                                }
                                else
                                {
                                    return syntaxError("invalid iteration assignment.");
                                }
                            }
                            else
                            {
                                return syntaxErrorUnexpected(";");
                            }
                        }
                        else
                        {
                            return syntaxError("invalid boolean expression in for loop.");
                        }
                    }
                    else
                    {
                        return syntaxErrorUnexpected(";");
                    }
                }
                else
                {
                    return syntaxError("invalid initialization expression in for loop.");
                }
            }
            else
            {
                return syntaxErrorUnexpected("(");
            }
        }
        else if (nextToken() == "while")
        {
            index++;
            if (nextType() == "L_PAREN")
            {
                index++;
                if (isBooleanExpression())
                {
                    if (nextType() == "R_PAREN")
                    {
                        addStatement();
                        index++;
                        validStart = true;
                    }
                    else
                    {
                        return syntaxErrorUnexpected(")");
                    }
                }
                else
                {
                    return syntaxError("invalid boolean expression for while loop.");
                }
            }
            else
            {
                return syntaxErrorUnexpected("(");
            }
        }
        else
        {
            return false;
        }
        // check for either statement or block statement
        if (validStart)
        {
            if (isStatementOrBlockStatement())
            {
                // addStatement();
                return true;
            }
            else
            {
                return syntaxError("invalid body statement following start of iteration loop.");
            }
        }
        else
        {
            // return syntaxError("invalid start of iteration loop.");
            return false;
        }
    }
    bool isSelectionStatement()
    {
        if (log)
        {
            printLog("selection statement");
        }
        // either if (<BOOLEAN_EXPRESSION>) <BODY> OR if (<BOOLEAN_EXPRESSION>) <BODY> else <BODY>
        // <BODY> is either <STATEMENT> OR <BLOCK_STATEMENT>
        if (nextToken() == "if")
        {
            index++;
            if (nextType() == "L_PAREN")
            {
                index++;
                if (isBooleanExpression())
                {
                    if (nextType() == "R_PAREN")
                    {
                        addStatement();
                        index++;
                        if (isStatementOrBlockStatement())
                        {
                            // addStatement();
                            if (nextToken() == "else")
                            {
                                addStatement();
                                index++;
                                if (isStatementOrBlockStatement())
                                {
                                    // addStatement();
                                    return true;
                                }
                                else
                                {
                                    return syntaxError("invalid body statement for else statement.");
                                }
                            }
                            else
                            {
                                return true;
                            }
                        }
                        else
                        {
                            return syntaxError("invalid body statement for if statement.");
                        }
                    }
                    else
                    {
                        return syntaxErrorUnexpected(")");
                    }
                }
                else
                {
                    return syntaxError("invalid boolean expression inside if statement.");
                }
            }
            else
            {
                return syntaxErrorUnexpected("(");
            }
        }
        else
        {
            // not a selection statement
            return false;
        }
    }
    bool isStatementOrBlockStatement()
    {
        if (log)
        {
            printLog("statement or block statement");
        }
        int preIdx = index;
        if (isBlockStatement())
        {
            return true;
        }
        else
        {
            if (errorFound)
            {
                return false;
            }
            index = preIdx;
            return isStatement();
        }
    }
    bool isPrintfStatement()
    {
        if (log)
        {
            printLog("printf statement");
        }
        // printf (<STRING>); OR printf(<STRING> <IDENTIFIER_LIST>);
        if (nextToken() == "printf")
        {
            index++;
            if (nextType() == "L_PAREN")
            {
                index++;
                if (isString())
                {
                    if (nextType() == "R_PAREN")
                    {
                        index++;
                        if (nextType() == "SEMICOLON")
                        {
                            addStatement();
                            index++;
                            return true;
                        }
                        else
                        {
                            return syntaxErrorUnexpected(";");
                        }
                    }
                    else if (nextType() == "COMMA")
                    {
                        index++;
                        if (isIdentifierOrIdentifierArrayList())
                        {
                            if (nextType() == "R_PAREN")
                            {
                                index++;
                                if (nextType() == "SEMICOLON")
                                {
                                    addStatement();
                                    index++;
                                    return true;
                                }
                                else
                                {
                                    return syntaxErrorUnexpected(";");
                                }
                            }
                            else
                            {
                                return syntaxErrorUnexpected(")");
                            }
                        }
                        else
                        {
                            return syntaxError("invalid variable list in printf statement.");
                        }
                    }
                    else
                    {
                        return true;
                    }
                }
                else
                {
                    return syntaxError("expected a string following printf. Got a(n)'" + nextType() + "' instead.");
                }
            }
            else
            {
                return syntaxErrorUnexpected("(");
            }
        }
        else
        {
            return false;
        }
    }
    bool isReturnStatement()
    {
        if (log)
        {
            printLog("return statement");
        }
        // return <EXPRESSION>; OR return <STRING>;
        if (nextToken() == "return")
        {
            index++;
            int preIdx = index; // trying multiple options, may backtrack
            if (isExpression())
            {
                if (nextType() == "SEMICOLON")
                {
                    addStatement();
                    index++;
                    return true;
                }
                else
                {
                    return syntaxErrorUnexpected(";");
                }
            }
            else
            {
                if (errorFound)
                {
                    return false;
                }
                index = preIdx;
                if (isString())
                {
                    if (nextType() == "SEMICOLON")
                    {
                        addStatement();
                        index++;
                        return true;
                    }
                    else
                    {
                        return syntaxErrorUnexpected(";");
                    }
                }
                else
                {
                    return syntaxError("invalid return value.");
                }
            }
        }
        else
        {
            // not a return statement, no error
            return false;
        }
    }
    bool isUserDefinedProcedureCallStatement()
    {
        // <IDENTIFIER> (<IDENTIFIER_LIST>); OR <IDENTIFIER> (<EXPRESSION>);
        if (log)
        {
            printLog("user defined procedure call statement");
        }
        bool validParams = false;
        if (isValidIdentifier())
        {
            index++;
            if (nextType() == "L_PAREN")
            {
                index++;
                int preIdx = index; // trying multiple options, may backtrack
                if (isIdentifierOrIdentifierArrayList())
                {
                    validParams = true;
                }
                else
                {
                    if (errorFound)
                    {
                        return false;
                    }
                    index = preIdx; // wasn't an identifier list, backtracking
                    if (isExpression())
                    {
                        validParams = true;
                    }
                    else
                    {
                        return syntaxError("invalid procedure parameters.");
                    }
                }

                if (validParams)
                {
                    if (nextType() == "R_PAREN")
                    {
                        index++;
                        if (nextType() == "SEMICOLON")
                        {
                            addStatement();
                            index++;
                            return true;
                        }
                        else
                        {
                            return syntaxErrorUnexpected(";");
                        }
                    }
                    else
                    {
                        return syntaxErrorUnexpected(")");
                    }
                }
                else
                {
                    return syntaxError("invalid procedure parameters.");
                }
            }
            else
            {
                return syntaxErrorUnexpected("(");
            }
        }
        else
        {
            // not a procedure call
            return false;
        }
    }
    bool isExpression()
    {
        if (log)
        {
            printLog("expression");
        }
        int preIdx = index;
        if (isBooleanExpression())
        {
            // conflict in which order we check
            if (nextType() != "SEMICOLON")
            {
                int boolIdx = index;
                index = preIdx;
                if (isNumericalExpression())
                {
                    // could have been numerical or boolean
                    if (index > boolIdx)
                    {
                        // the possible numerical expression was longer
                        return true;
                    }
                    else
                    {
                        // the possible boolean expression was longer
                        index = boolIdx;
                        return true;
                    }
                }
                else
                {
                    // definitely a boolean expression, not numerical
                    return true;
                }
            }
            return true;
        }
        else
        {
            if (errorFound)
            {
                return false;
            }
            index = preIdx;
            if (isNumericalExpression())
            {
                return true;
            }
            else
            {
                index = preIdx;
                return false;
            }
        }
    }
    bool isInitializationExpression()
    {
        if (log)
        {
            printLog("initialization expression");
        }
        // combined declaration and assignment
        if (isValidIdentifier())
        {
            index++;
            if (nextType() == "ASSIGNMENT_OPERATOR")
            {
                index++;
                int prevIdx = index;
                if (isExpression())
                {
                    return true;
                }
                else
                {
                    if (errorFound)
                    {
                        return false;
                    }
                    index = prevIdx;
                    return isString() ? true : syntaxError("invalid right hand side of assignment operator.");
                }
            }
            else
            {
                return syntaxErrorUnexpected("=");
            }
        }
        else
        {
            return syntaxError("invalid identifier '" + nextToken() + "' in for loop variable initialization.");
        }
    }
    bool isBooleanExpression()
    {
        if (log)
        {
            printLog("boolean expression");
        }
        // <BOOLEAN_OPERAND> OR <BOOLEAN_OPERAND> <BOOLEAN_OPERATOR> <BOOLEAN_EXPRESSION>
        // OR (<BOOLEAN_OPERAND>) OR <BOOLEAN_NOT> <BOOLEAN_EXPRESSION> OR <COMPARISON_EXPRESSION>
        int preIdx = index; // possible backtracking
        if (isComparisonExpression())
        {
            return true;
        }
        else
        {
            if (errorFound)
            {
                return false;
            }
            index = preIdx;
            if (isBooleanOperand())
            {
                if (isBooleanOperator())
                {
                    index++;
                    return isBooleanExpression() ? true : syntaxError("invalid boolean expression following boolean operator.");
                }
                else
                {
                    return true;
                }
            }
            else
            {
                if (errorFound)
                {
                    return false;
                }
                if (nextType() == "L_PAREN")
                {
                    index++;
                    if (isBooleanExpression())
                    {
                        if (nextType() == "R_PAREN")
                        {
                            index++;
                            if (isBooleanOperator())
                            {
                                index++;
                                return isBooleanExpression() ? true : syntaxError("invalid boolean expression following boolean operator.");
                            }
                            else
                            {
                                return true;
                            }
                        }
                        else
                        {
                            if (isNumericalOperator())
                            {
                                // may have incorrectly assumed it to be a boolean expression?
                                return false;
                            }
                            return syntaxErrorUnexpected(")");
                        }
                    }
                    else
                    {
                        return syntaxError("invalid boolean expression inside parentheses.");
                    }
                }
                else if (nextType() == "BOOLEAN_NOT")
                {
                    index++;
                    return isBooleanExpression() ? true : syntaxError("invalid boolean expression following boolean not.");
                }
                else
                {
                    // not a boolean expression
                    return false;
                }
            }
        }
    }
    bool isBooleanOperand()
    {
        if (log)
        {
            printLog("boolean operand");
        }
        if (nextToken() == "true")
        {
            index++;
            return true;
        }
        else if (nextToken() == "false")
        {
            index++;
            return true;
        }
        else
        {
            int prevIdx = index;
            if (isUserDefinedFunction())
            {
                return true;
            }
            else
            {
                if (errorFound)
                {
                    return false;
                }
                index = prevIdx;
                if (isValidIdentifier())
                {
                    index++;
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
    }
    bool isBooleanOperator()
    {
        if (log)
        {
            cout << "checking for boolean operator" << endl;
            cout << "current token: " << nextToken() << endl;
        }
        vector<string> booleanOperators = {"BOOLEAN_OR", "BOOLEAN_AND"};
        string curr = nextType();
        for (int i = 0; i < booleanOperators.size(); i++)
        {
            if (curr == booleanOperators.at(i))
            {
                return true;
            }
        }
        return false;
    }
    bool isComparisonExpression()
    {
        if (log)
        {
            printLog("comparison expression");
        }
        // <NUMERICAL_OPERAND> <COMPARISON_OPERATOR> <NUMERICAL_OPERAND>
        if (isNumericalExpression())
        {
            if (isComparisonOperator())
            {
                index++;
                return isNumericalExpression();
                // return isNumericalOperand() ? true : syntaxError("invalid numerical operand following start of comparison expression.");
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    bool isComparisonOperator()
    {
        if (log)
        {
            cout << "checking for comparison operator" << endl;
            cout << "current token: " << nextToken() << endl;
        }
        vector<string> comparisonOperators = {"LT", "LT_EQUAL", "GT", "GT_EQUAL", "BOOLEAN_EQUAL", "BOOLEAN_NOT_EQUAL"};
        string curr = nextType();
        for (int i = 0; i < comparisonOperators.size(); i++)
        {
            if (curr == comparisonOperators.at(i))
            {
                return true;
            }
        }
        return false;
    }
    bool isIterationAssignment()
    {
        if (log)
        {
            printLog("iteration assignment");
        }
        if (isValidIdentifier())
        {
            index++;
            if (nextType() == "ASSIGNMENT_OPERATOR")
            {
                index++;
                int prevIdx = index;
                if (isExpression())
                {
                    return true;
                }
                else
                {
                    if (errorFound)
                    {
                        return false;
                    }
                    index = prevIdx;
                    return isString() ? true : syntaxError("invalid right hand side of assignment operator.");
                }
            }
            else
            {
                return syntaxErrorUnexpected("=");
            }
        }
        else
        {
            return false;
        }
    }
    bool isNumericalExpression()
    {
        if (log)
        {
            printLog("numerical expression");
        }
        // <NUMERICAL_OPERAND> OR <NUMERICAL_OPERAND> <NUMERICAL_OPERATOR> <NUMERICAL_EXPRESSION>
        // OR (<NUMERICAL_EXPRESSION>)
        if (isNumericalOperand())
        {
            if (isNumericalOperator())
            {
                index++;
                return isNumericalExpression();
                // syntaxError("invalid numerical expression following numerical operator.");
            }
            else
            {
                return true;
            }
        }
        else
        {
            if (errorFound)
            {
                return false;
            }
            if (nextType() == "L_PAREN")
            {
                index++;
                if (isNumericalExpression())
                {
                    if (nextType() == "R_PAREN")
                    {
                        index++;
                        if (isNumericalOperator())
                        {
                            index++;
                            return isNumericalExpression();
                        }
                        else
                        {
                            return true;
                        }
                    }
                    else
                    {
                        // return syntaxErrorUnexpected(")");
                        return false;
                    }
                }
                else
                {
                    return false;
                    // return syntaxError("invalid numerical expression inside parentheses.");
                }
            }
            else
            {
                // not a numerical expression
                return false;
            }
        }
    }
    bool isNumericalOperand()
    {
        if (log)
        {
            printLog("numerical operand");
        }
        if (nextType() == "INTEGER")
        {
            index++;
            return true;
        }
        else
        {
            int prevIdx = index;
            if (isUserDefinedFunction())
            {
                return true;
            }
            else
            {
                if (errorFound)
                {
                    return false;
                }
                index = prevIdx;
                if (isGetcharFunction())
                {
                    return true;
                }
                else
                {
                    if (errorFound)
                    {
                        return false;
                    }
                    if (isSizeofFunction())
                    {
                        return true;
                    }
                    else
                    {
                        if (errorFound)
                        {
                            return false;
                        }
                        if (isVariableOrVariableArray())
                        {
                            return true;
                        }
                        else
                        {
                            if (errorFound)
                            {
                                return false;
                            }
                            prevIdx = index;
                            return isString();
                        }
                    }
                }
            }
        }
    }
    bool isNumericalOperator()
    {
        if (log)
        {
            printLog("numerical operator");
        }
        vector<string> numericalOperators = {"PLUS", "MINUS", "ASTERISK", "DIVIDE", "MODULO", "CARET"};
        string curr = nextType();
        for (int i = 0; i < numericalOperators.size(); i++)
        {
            if (curr == numericalOperators.at(i))
            {
                return true;
            }
        }
        return false;
    }
    bool isUserDefinedFunction()
    {
        if (log)
        {
            printLog("user defined function");
        }
        if (isValidIdentifier())
        {
            index++;
            if (nextToken() == "(")
            {
                index++;
                int preIdx = index;
                if (isIdentifierOrIdentifierArrayList())
                {
                    if (nextType() == "R_PAREN")
                    {
                        index++;
                        return true;
                    }
                    else
                    {
                        return syntaxErrorUnexpected(")");
                    }
                }
                else
                {
                    if (errorFound)
                    {
                        return false;
                    }
                    index = preIdx;
                    if (isExpression())
                    {
                        if (errorFound)
                        {
                            return false;
                        }
                        return nextType() == "R_PAREN" ? true : syntaxErrorUnexpected(")");
                    }
                    else
                    {
                        return syntaxError("invalid arguments to user defined function.");
                    }
                }
            }
            else
            {
                // should this return syntax error?
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    bool isString()
    {
        if (log)
        {
            cout << "checking for string" << endl;
            cout << "current token: " << nextToken() << endl;
        }
        if (nextType() == "DOUBLE_QUOTE")
        {
            index++;
            if (nextType() == "STRING")
            {
                index++;
                if (nextType() == "DOUBLE_QUOTE")
                {
                    index++;
                    return true;
                }
                else
                {
                    return syntaxError("improperly closed string.");
                }
            }
            else
            {
                return syntaxErrorUnexpected("string");
            }
        }
        else if (nextType() == "SINGLE_QUOTE")
        {
            index++;
            if (nextType() == "STRING")
            {
                index++;
                if (nextType() == "SINGLE_QUOTE")
                {
                    index++;
                    return true;
                }
                else
                {
                    return syntaxError("improperly closed string.");
                }
            }
            else
            {
                return syntaxErrorUnexpected("string");
            }
        }
        else
        {
            return false;
        }
    }
    bool isGetcharFunction()
    {
        if (log)
        {
            printLog("getchar function");
        }
        if (nextToken() == "getchar")
        {
            index++;
            if (nextType() == "L_PAREN")
            {
                index++;
                if (nextToken() == "void")
                {
                    index++;
                    if (nextType() == "R_PAREN")
                    {
                        index++;
                        return true;
                    }
                    else
                    {
                        return syntaxErrorUnexpected(")");
                    }
                }
                else
                {
                    return syntaxError("Unexpected parameter for getchar function, '" + nextToken() + "'. Expected void.");
                }
            }
            else
            {
                return syntaxErrorUnexpected("(");
            }
        }
        else
        {
            return false;
        }
    }
    bool isSizeofFunction()
    {
        if (log)
        {
            printLog("sizeof function");
        }
        if (nextToken() == "sizeof")
        {
            index++;
            if (nextType() == "L_PAREN")
            {
                index++;
                if (isVariableOrVariableArray())
                {
                    if (nextType() == "R_PAREN")
                    {
                        index++;
                        return true;
                    }
                    else
                    {
                        return syntaxErrorUnexpected(")");
                    }
                }
                else
                {
                    return syntaxError("Invalid argument for sizeof function.");
                }
            }
            else
            {
                return syntaxErrorUnexpected("(");
            }
        }
        else
        {
            return false;
        }
    }
    bool isVariableOrVariableArray()
    {
        if (log)
        {
            printLog("variable or variable array");
        }
        if (isValidIdentifier())
        {
            index++;
            if (nextType() == "L_BRACKET")
            {
                index++;
                if (nextType() == "INTEGER") // checking for [-integer]
                {
                    if (nextToken().at(0) == '-')
                    {
                        index++;
                        if (nextType() == "R_BRACKET")
                        {
                            index++;
                            return syntaxError("cannot use a negative integer as an array index/size.");
                        }
                    }
                }
                if (isNumericalExpression())
                {
                    if (nextType() == "R_BRACKET")
                    {
                        index++;
                        return true;
                    }
                    return syntaxError("missing ']' to close array.");
                }
                else
                {
                    return syntaxError("invalid array index/size, must be a valid numerical expression.");
                }
            }
            else
            {
                return true;
            }
        }
        else
        {
            return false;
        }
    }
};

#endif