#include "Interpreter.h"
#include <stdexcept>
#include <stack>

using namespace std;

// -------------------------
// Statement Type Helper
// -------------------------
// This makes executeBlock() cleaner by categorizing statements based on their AST node labels.
enum class StatementKind
{
    ASSIGNMENT,
    PRINTF,
    IF,
    WHILE,
    FOR,
    RETURN,
    CALL,
    BEGIN_BLOCK,
    END_BLOCK,
    DECLARATION,
    OTHER
};
// static helper function to determine the kind of statement based on the AST node label
static StatementKind getStatementKind(Node* node)
{
    if (node == nullptr)
    {
        return StatementKind::OTHER;
    }

    if (node->label == "ASSIGNMENT")
    {
        return StatementKind::ASSIGNMENT;
    }
    else if (node->label == "PRINTF")
    {
        return StatementKind::PRINTF;
    }
    else if (node->label == "IF")
    {
        return StatementKind::IF;
    }
    else if (node->label == "WHILE")
    {
        return StatementKind::WHILE;
    }
    else if (node->label == "FOR EXPRESSION 1")
    {
        return StatementKind::FOR;
    }
    else if (node->label == "RETURN")
    {
        return StatementKind::RETURN;
    }
    else if (node->label == "CALL")
    {
        return StatementKind::CALL;
    }
    else if (node->label == "BEGIN BLOCK")
    {
        return StatementKind::BEGIN_BLOCK;
    }
    else if (node->label == "END BLOCK")
    {
        return StatementKind::END_BLOCK;
    }
    else if (node->label == "DECLARATION")
    {
        return StatementKind::DECLARATION;
    }

    return StatementKind::OTHER;
}
// helper function to decode escaped character literals 
static char decodeCharLiteral(const string& text)
{
    if (text == "\\n")
    {
        return '\n';
    }
    if (text == "\\t")
    {
        return '\t';
    }
    if (text == "\\0")
    {
        return '\0';
    }
    if (text.size() >= 2 && text[0] == '\\' && text[1] == 'x')
    {
        string hexStr = text.substr(2);
        if (hexStr.empty()) return '\0';
        return (char)stoi(hexStr, nullptr, 16);
    }

    if (text.empty())
    {
        return '\0';
    }
    if (text[0] == '\\' && text.size() >= 2) {
        return text[1];
    }

    return text[0];
}

// -------------------------
// Constructor / Run
// -------------------------
// Initializes the interpreter with the AST root, symbol table head, and output stream.
Interpreter::Interpreter(Node* astRoot, SymbolNode* symbolTableHead, ostream& out)
    : astRoot(astRoot),
      symbolTableHead(symbolTableHead),
      out(out),
      currentScope(0),
      isReturning(false),
      lastReturnValue(0) 
      {
      }
// Starts the interpretation process by finding the main procedure and executing its block.
void Interpreter::run()
{
    Node* mainNode = findMainProcedure();

    if (mainNode == nullptr)
    {
        throw runtime_error("main procedure not found");
    }

    SymbolNode* mainSymbol = findFunctionSymbol("main");

    if (mainSymbol != nullptr)
    {
        currentScope = mainSymbol->scope;
    }

    Node* mainBlock = getNextLine(mainNode);

    if (mainBlock == nullptr || mainBlock->label != "BEGIN BLOCK")
    {
        throw runtime_error("main procedure has no BEGIN BLOCK");
    }

    executeBlock(mainBlock);
}

// -------------------------
// AST Navigation
// -------------------------
// helper function to find the main procedure declaration in the AST
Node* Interpreter::findMainProcedure()
{
    Node* cur = astRoot;

    while (cur != nullptr)
    {
        if (cur->label == "DECLARATION" && cur->text == "main")
        {
            return cur;
        }

        cur = getNextLine(cur);
    }

    return nullptr;
}
// helper function to get the next line in the AST
Node* Interpreter::getNextLine(Node* current)
{
    while (current != nullptr)
    {
        if (current->child != nullptr)
        {
            return current->child;
        }
        current = current->sibling;
    }
    return nullptr;
}

// -------------------------
// Block Execution
// -------------------------
// Main funtion to execute a block of statements. It iterates through the statements in the block and executes them based on their type.
void Interpreter::executeBlock(Node* blockNode)
{
    if (blockNode == nullptr || blockNode->label != "BEGIN BLOCK")
    {
        return;
    }
    Node* current = getNextLine(blockNode);
    // Keep executing statements until the block ends or a return is hit.
    while (current != nullptr && !isReturning)
    {
        StatementKind kind = getStatementKind(current);

        switch (kind)
        {
            case StatementKind::ASSIGNMENT:
                executeAssignment(current);
                current = getNextLine(current);
                break;

            case StatementKind::PRINTF:
                // Expected behavior is to evaluate the printf arguments and print them to the output stream. 
                // For example, if the AST represents printf("x = %d\n", x); 
                // then we would evaluate the string literal "x = %d\n" and the variable x, and then print the formatted string with x's value. 
                executePrintf(current);
                current = getNextLine(current);
                break;

            case StatementKind::IF:
            {
                // Expected behavior
                // executeIf(current) should evaluate the condition.
                // If true, it executes the if block.
                // If false, it skips to the else block if one exists.

                // feel free to change this if you want
                // We need to find the then block and else block (if it exists) so we know where to jump after executing the if statement.
                Node* thenBlock = getNextLine(current);
                Node* afterIf = skipBlock(thenBlock);

                if (afterIf != nullptr && afterIf->label == "ELSE")
                {
                    Node* elseBlock = getNextLine(afterIf);
                    afterIf = skipBlock(elseBlock);
                }

                executeIf(current);
                current = afterIf;
                break;
            }

            case StatementKind::WHILE:
            {
                // Expected behavior
                // executeWhile(current) should repeatedly evaluate the condition.
                // While the condition is true, it executes the loop body.

                // feel free to change this if you want
                // find the while body block so we know where to jump back to after each iteration and where to jump after the loop is done.
                Node* bodyBlock = getNextLine(current);
                Node* afterWhile = skipBlock(bodyBlock);
                // execute the while loop.
                executeWhile(current);
                // continue after while loop when condition is false
                current = afterWhile;
                break;
            }

            case StatementKind::FOR:
            {
                // Expected behavior
                // executeFor(current) should run
                //   expression 1 once as initialization
                //   expression 2 as the condition
                //   expression 3 after every loop body execution

                // feel free to change this if you want

                // These find the statement after the whole for loop,
                // so executeBlock knows where to continue once executeFor finishes.
                Node* expr2 = getNextLine(current);
                Node* expr3 = getNextLine(expr2);
                Node* bodyBlock = getNextLine(expr3);
                Node* afterFor = skipBlock(bodyBlock);

                // execute the for loop.
                executeFor(current);
                // continue after for loop when condition is false  
                current = afterFor;
                break;
            }

            case StatementKind::RETURN:
                executeReturn(current);
                current = getNextLine(current);
                break;

            case StatementKind::CALL:
            {
                Node* functionNameNode = current->sibling;

                if (functionNameNode == nullptr)
                {
                    throw runtime_error("Invalid function/procedure call");
                }

                Node* openParenNode = functionNameNode->sibling;
                vector<int> args = evaluateArgumentList(openParenNode);

                executeFunction(functionNameNode->text, args);

                current = getNextLine(current);
                break;
            }

            case StatementKind::BEGIN_BLOCK:
            {
                Node* afterBlock = skipBlock(current);
                executeBlock(current);
                current = afterBlock;
                break;
            }

            case StatementKind::END_BLOCK:
                return;

            case StatementKind::DECLARATION:
                current = getNextLine(current);
                break;

            case StatementKind::OTHER:
            default:
                out << "Unknown node: " << current->label << endl;
                current = getNextLine(current);
                break;
        }
    }
}

//skips over a block of statements and returns the node after the block. Used for if-else and loop statements.
Node* Interpreter::skipBlock(Node* beginBlockNode)
{
    if (beginBlockNode == nullptr || beginBlockNode->label != "BEGIN BLOCK")
    {
        return nullptr;
    }
    int depth = 1;
    Node* current = getNextLine(beginBlockNode);
    while (current != nullptr)
    {
        if (current->label == "BEGIN BLOCK")
        {
            depth++;
        }
        else if (current->label == "END BLOCK")
        {
            depth--;
            if (depth == 0)
            {
                return getNextLine(current);
            }
        }
        current = getNextLine(current);
    }
    return nullptr;
}
// -------------------------
// Symbol Lookup
// -------------------------
// helper function to find a funtion or procedure symbol by name in the symbol table
SymbolNode* Interpreter::findFunctionSymbol(const string& name)
{
    for (SymbolNode* cur = symbolTableHead; cur != nullptr; cur = cur->next)
    {
        if (cur->name == name && (cur->identifierType == "function" || cur->identifierType == "procedure"))
        {
            return cur;
        }
    }

    return nullptr;
}


// helper function to look up a symbol by name and current scope, throwing an error if not found
SymbolNode* Interpreter::lookupSymbol(const string& name)
{
    SymbolNode* symbol = findSymbol(symbolTableHead, name, currentScope);

    if (symbol == nullptr)
    {
        throw runtime_error("Undefined identifier: " + name);
    }
    return symbol;
}

// -------------------------
// Integer Helpers
// -------------------------
// helper function to get the integer value of a symbol
int Interpreter::getSymbolIntValue(SymbolNode* symbol)
{
    if (symbol == nullptr) {
        throw runtime_error("NULL symbol");
    } else if (symbol->datatype != "int") {
        throw runtime_error("Not an int: " + symbol->name);
    }
    return getIntValue(symbol);
}
// helper function to get the integer value of a variable by name
int Interpreter::getVariableIntValue(const string& name)
{
    SymbolNode* symbol = lookupSymbol(name);
    return getSymbolIntValue(symbol);
}
// helper function to assign an integer value to a symbol
void Interpreter::assignIntValue(SymbolNode* symbol, int index, int value)
{
    if (symbol == nullptr)
    {
        throw runtime_error("NULL symbol");
    } else if (symbol->datatype != "int"){
        throw runtime_error("Not an int: " + symbol->name);
    } else if (index >= 0) {
        setIntArrayValue(symbol, index, value);
    } else {
        setIntValue(symbol, value);
    }
}

// -------------------------
// Character Helpers
// -------------------------
// same ideas as integer helpers but for char type
char Interpreter::getSymbolCharValue(SymbolNode* symbol)
{
    if (symbol == nullptr)
    {
        throw runtime_error("NULL symbol");
    } else if (symbol->datatype != "char"){
        throw runtime_error("Not a char : " + symbol->name);
    }
    return getCharValue(symbol);
}

char Interpreter::getVariableCharValue(const string& name)
{
    SymbolNode* symbol = lookupSymbol(name);
    return getSymbolCharValue(symbol);
}

void Interpreter::assignCharValue(SymbolNode* symbol, int index, char value)
{
    if (symbol == nullptr) {
        throw runtime_error("Cannot assign to null symbol");
    } else if (symbol->datatype != "char"){
        throw runtime_error("Expected char variable: " + symbol->name);
    } if (index >= 0) {
        setCharArrayValue(symbol, index, value);
    }
    else {
        setCharValue(symbol, value);
    }
}
// this is for test program 2 mainly 
char Interpreter::evaluateCharExpression(Node* exprNode)
{
    // if it null or empty, throw an error since we can't evaluate it
    if (exprNode == nullptr)
    {
        throw runtime_error("Empty char expression");
    }
    // Handle character literals like 'a' or '\n'
    if (exprNode->text == "'" && exprNode->sibling != nullptr && exprNode->sibling->label == "ESCAPED_CHARACTER")
    {
        return decodeCharLiteral(exprNode->sibling->text);
    }
    // handle the case where it's just a single character literal without the surrounding single quotes
    if (exprNode->label == "ESCAPED_CHARACTER")
    {
        return decodeCharLiteral(exprNode->text);
    }
    // If it's an identifier, look up its value. If it's an array, evaluate the index and get the character at that index.
    if (exprNode->label == "IDENTIFIER")
    {
        SymbolNode* symbol = lookupSymbol(exprNode->text);

        if (symbol->isArray)
        {
            Node* bracket = exprNode->sibling;

            int index = evaluateExpression(bracket->sibling);
            return getCharArrayValue(symbol, index);
        }

        return getSymbolCharValue(symbol);
    }
    // if it a integer expression, evaluate it and convert to char
    return static_cast<char>(evaluateExpression(exprNode));
}


// Returns true when evaluateExpression should stop reading expression nodes
static bool isExpressionStop(Node* node)
{
    if (node == nullptr)
    {
        return true;
    }

    return node->label == "ASSIGN" ||
           node->label == "BEGIN BLOCK" ||
           node->label == "END BLOCK" ||
           node->text == "]" ||
           node->text == ")" ||
           node->text == ",";
}
// helper function to check if a node represents an operator
static bool isOperator(Node* node)
{
    if (node == nullptr)
    {
        return false;
    }

    return node->text == "!" ||
           node->text == "+" ||
           node->text == "-" ||
           node->text == "*" ||
           node->text == "/" ||
           node->text == "%" ||
           node->text == "<" ||
           node->text == "<=" ||
           node->text == ">" ||
           node->text == ">=" ||
           node->text == "==" ||
           node->text == "!=" ||
           node->text == "&&" ||
           node->text == "||";
}
// helper function to apply an operator to two integer operands and return the result
static int applyOperator(const string& op, int left, int right)
{
    if (op == "+")
    {
        return left + right;
    }
    else if (op == "-")
    {
        return left - right;
    }
    else if (op == "*")
    {
        return left * right;
    }
    else if (op == "/")
    {

        return left / right;
    }
    else if (op == "%")
    {

        return left % right;
    }
    else if (op == "<")
    {
        return left < right;
    }
    else if (op == "<=")
    {
        return left <= right;
    }
    else if (op == ">")
    {
        return left > right;
    }
    else if (op == ">=")
    {
        return left >= right;
    }
    else if (op == "==")
    {
        return left == right;
    }
    else if (op == "!=")
    {
        return left != right;
    }
    else if (op == "&&")
    {
        return left && right;
    }
    else if (op == "||")
    {
        return left || right;
    }

    throw runtime_error("Unknown operator: " + op);
}

// Evaluates the postfix expression using a stack.
// Chars are treated as Ascii values.
int Interpreter::evaluateExpression(Node* exprNode)
{
    stack<int> values;
    Node* current = exprNode;
    // Iterate through the expression nodes until we reach a stop node.
    while (current != nullptr && !isExpressionStop(current))
    {
        // if it int treat it as an integer literal and push its value onto the stack. 
        if (current->label == "INTEGER")
        {
            values.push(stoi(current->text));
        }
        // if it a boolean literal, push 1 for true and 0 for false onto the stack
        else if (current->text == "TRUE")
        {
            values.push(1);
        }
        else if (current->text == "FALSE")
        {
            values.push(0);
        }
        // if it a char literal, decode it and push its ASCII value onto the stack
        else if (current->label == "ESCAPED_CHARACTER")
        {
            values.push(static_cast<int>(decodeCharLiteral(current->text)));
        }
        // Identifier can mean a variable, array access, or function call.
        else if (current->label == "IDENTIFIER")
        {
            // function call if followed by an open parenthesis
            if (current->sibling != nullptr && current->sibling->text == "(")
            {
                vector<int> args = evaluateArgumentList(current->sibling);
                int result = executeFunction(current->text, args);
                values.push(result);

                while (current != nullptr && current->text != ")")
                {
                    current = current->sibling;
                }
            }
            else
            {
                // variable or array access
                SymbolNode* symbol = lookupSymbol(current->text);
                // if it's an array access, evaluate the index and get the value at that index
                if (current->sibling != nullptr && current->sibling->text == "[")
                {
                    Node* indexStart = current->sibling->sibling;
                    int index = evaluateExpression(indexStart);

                    if (symbol->datatype == "int")
                    {
                        values.push(getIntArrayValue(symbol, index));
                    }
                    else if (symbol->datatype == "char")
                    {
                        values.push(static_cast<int>(getCharArrayValue(symbol, index)));
                    }
                    else
                    {
                        throw runtime_error("I have not added this array type: " + symbol->datatype);
                    }
                    while (current != nullptr && current->text != "]")
                    {
                        current = current->sibling;
                    }
                }
                // if it's a simple variable, get its value and push it onto the stack
                else
                {
                    if (symbol->datatype == "int")
                    {
                        values.push(getSymbolIntValue(symbol));
                    }
                    else if (symbol->datatype == "char")
                    {
                        values.push(static_cast<int>(getSymbolCharValue(symbol)));
                    }
                    else
                    {
                        throw runtime_error("I have not added this variable type: " + symbol->datatype);
                    }
                }
            }
        }
        // if it's a unary operator like !, pop the top value, apply the operator, and push the result back onto the stack
        else if (current->text == "!")
        {
            if (values.empty())
            {
                throw runtime_error("Missing operand for !");
            }

            int value = values.top();
            values.pop();

            values.push(!value);
        }
        // if it's a binary operator, pop the top two values, apply the operator, and push the result back onto the stack
        else if (isOperator(current))
        {
            if (values.size() < 2)
            {
                throw runtime_error("Missing operands for operator " + current->text);
            }

            int right = values.top();
            values.pop();

            int left = values.top();
            values.pop();

            values.push(applyOperator(current->text, left, right));
        }

        current = current->sibling;
    }

    if (values.empty())
    {
        throw runtime_error("Empty expression or something went very wrong");
    }
    // The final result should be the only value left on the stack. Return it as the result of evaluating the expression.
    return values.top();
}

// -------------------------
// Assignment Execution
// -------------------------
// Executes assignment statements for int variables, char variables,
// array elements, and string assignment into char arrays.
void Interpreter::executeAssignment(Node* assignmentNode)
{
    // The target of the assignment is the first sibling of the ASSIGNMENT node.
    Node* target = assignmentNode->sibling;

    if (target == nullptr)
    {
        throw runtime_error("Invalid assignment");
    }
    // Look up the symbol for the target variable.
    SymbolNode* symbol = lookupSymbol(target->text);
    // Check if the assignment is to an array element by looking for a "[" sibling. If so, evaluate the index expression.
    int index = -1;
    Node* exprStart = target->sibling;

    if (exprStart != nullptr && exprStart->text == "[")
    {
        index = evaluateExpression(exprStart->sibling);

        while (exprStart != nullptr && exprStart->text != "]")
        {
            exprStart = exprStart->sibling;
        }

        if (exprStart != nullptr)
        {
            exprStart = exprStart->sibling;
        }
    }
    //handle ints
    if (symbol->datatype == "int")
    {
        int value = evaluateExpression(exprStart);
        assignIntValue(symbol, index, value);
    }
    // handles char assignment
    else if (symbol->datatype == "char")
    {
        // If it's a char array and it a whole string 
        if (symbol->isArray && index == -1)
        {
            int arrayIndex = 0;
            Node* cur = exprStart;
            while (cur != nullptr && !isExpressionStop(cur))
            {
                if (cur->label == "STRING")
                {
                    string text = cur->text;
                    for (size_t i = 0; i < text.length(); i++)
                    {
                        if (text[i] == '\\' && i + 1 < text.length())
                        {
                            char c;
                            if (text[i+1] == 'n') { c = '\n'; i++; }
                            else if (text[i+1] == 't') { c = '\t'; i++; }
                            else if (text[i+1] == '0') { c = '\0'; i++; }
                            else if (text[i+1] == 'x' && i + 2 < text.length()) {
                                string hexStr = "";
                                i += 2; // skip \x
                                auto isHex = [](char ch) {
                                    return (ch >= '0' && ch <= '9') ||
                                           (ch >= 'A' && ch <= 'F') ||
                                           (ch >= 'a' && ch <= 'f');
                                };
                                if (i < text.length() && isHex(text[i])) {
                                    hexStr += text[i];
                                    if (i + 1 < text.length() && isHex(text[i+1])) {
                                        hexStr += text[i+1];
                                        i++;
                                    }
                                }
                                if (!hexStr.empty()) c = (char)stoi(hexStr, nullptr, 16);
                                else c = '\0';
                            }
                            else { c = text[i+1]; i++; }

                            if (arrayIndex < symbol->arraySize)
                                assignCharValue(symbol, arrayIndex++, c);
                        }
                        else
                        {
                            if (arrayIndex < symbol->arraySize)
                                assignCharValue(symbol, arrayIndex++, text[i]);
                        }
                    }
                }
                else if (cur->label == "ESCAPED_CHARACTER")
                {
                    char c = decodeCharLiteral(cur->text);
                    if (arrayIndex < symbol->arraySize)
                        assignCharValue(symbol, arrayIndex++, c);
                }
                cur = cur->sibling;
            }
            if (arrayIndex < symbol->arraySize)
            {
                assignCharValue(symbol, arrayIndex, '\0');
            }
        }
        // Handles a standalone escaped character node.
        else 
        {
            // normal char assignment, evaluate the char expression and assign the resulting char value to the variable or array element.
            char value = evaluateCharExpression(exprStart);
            assignCharValue(symbol, index, value);
        }
    }
    else
    {
        throw runtime_error("Did not add this type yet: " + symbol->datatype);
    }
}


// Executes a return statement inside a function.
// It stores the return value and tells executeBlock() to stop running the function body.
void Interpreter::executeReturn(Node* returnNode)
{
    if (returnNode == nullptr)
    {
        return;
    }
    lastReturnValue = evaluateExpression(returnNode->sibling);
    isReturning = true;
}

// Evaluates the arguments passed into a function call.
// This returns a vector containing the evaluated argument values.
// have not added arrays
vector<int> Interpreter::evaluateArgumentList(Node* openParenNode)
{
    vector<int> args;
    if (openParenNode == nullptr || openParenNode->text != "(")
    {
        return args;
    }

    Node* current = openParenNode->sibling;
    // Iterate through the argument nodes until we reach the closing parenthesis, evaluating each argument expression and adding it to the args vector.
    while (current != nullptr && current->text != ")")
    {
        args.push_back(evaluateExpression(current));
        while (current != nullptr && current->text != "," && current->text != ")")
        {
            current = current->sibling;
        }
        if (current != nullptr && current->text == ",")
        {
            current = current->sibling;
        }
    }
    // this returns the vector of evaluated argument values to be used in the function call execution.
    return args;
}

// Executes a function or procedure call.
// It switches into the function's scope, assigns arguments to parameters,
// runs the function body, then restores the old scope.
int Interpreter::executeFunction(const string& funcName, const vector<int>& args)
{
    // Look up the function/procedure symbol in the symbol table to get its scope and parameter information.
    SymbolNode* functionSymbol = findFunctionSymbol(funcName);

    if (functionSymbol == nullptr)
    {
        throw runtime_error("Undefined function/procedure: " + funcName);
    }
    // Find the function/procedure declaration node in the AST by matching the name. This is needed to get the body block of the function.
    Node* functionNode = astRoot;
    while (functionNode != nullptr)
    {
        if (functionNode->label == "DECLARATION" && functionNode->text == funcName)
        {
            break;
        }
        functionNode = getNextLine(functionNode);
    }

    if (functionNode == nullptr)
    {
        throw runtime_error("could not find function/procedure: " + funcName);
    }
    // Save the current scope and return state so we can restore it after the function call.
    int oldScope = currentScope;
    bool oldIsReturning = isReturning;
    int oldReturnValue = lastReturnValue;

    currentScope = functionSymbol->scope;
    isReturning = false;
    lastReturnValue = -1;
    // Assign the argument values to the function's parameters in the new scope.
    // We iterate through the parameter list and assign each argument value to the corresponding parameter variable.
    ParameterNode* param = functionSymbol->parameterList;
    size_t argIndex = 0;
    while (param != nullptr && argIndex < args.size())
    {
        SymbolNode* paramSymbol = findSymbol(symbolTableHead, param->name, currentScope);

        if (paramSymbol == nullptr)
        {
            throw runtime_error("Parameter symbol not found: " + param->name);
        }
        if (param->datatype == "int")
        {
            assignIntValue(paramSymbol, -1, args[argIndex]);
        }
        else if (param->datatype == "char")
        {
            assignCharValue(paramSymbol, -1, static_cast<char>(args[argIndex]));
        }
        else
        {
            throw runtime_error("Unsupported parameter type: " + param->datatype);
        }

        param = param->next;
        argIndex++;
    }

    // Now we execute the function body block. We find the BEGIN BLOCK node that represents the function body and call executeBlock() to run it.
    Node* bodyBlock = getNextLine(functionNode);
    executeBlock(bodyBlock);

    int result = lastReturnValue;
    // Restore the old scope and return state after the function call is complete.
    currentScope = oldScope;
    isReturning = oldIsReturning;
    lastReturnValue = oldReturnValue;
    // Return the result of the function call
    return result;
}




void Interpreter::executeIf(Node *ifNode)
{
    Node *exprStart = ifNode->sibling;

    if (exprStart == nullptr)
    {
        throw runtime_error("Missing contional expression");
    }

    bool result = evaluateExpression(exprStart);

    Node *thenBlock = getNextLine(ifNode);

    if (result)
    {
        Node *ifBlock = getNextLine(exprStart);
        executeBlock(ifBlock);
        return;
    }
    else
    {
        // check if else block exists, if so, execute
        Node *afterIf = skipBlock(thenBlock);
        if (afterIf != nullptr && afterIf->label == "ELSE")
        {
            Node *elseBlock = getNextLine(afterIf);
            executeBlock(elseBlock);
            return;
        }
        return;
    }
}


void Interpreter::executeWhile(Node* whileNode)
{
    if (whileNode == nullptr)
    {
        return;
    }

    Node* bodyBlock = getNextLine(whileNode);

    while (!isReturning && evaluateExpression(whileNode->sibling))
    {
        executeBlock(bodyBlock);
    }
}


void Interpreter::executeFor(Node* forNode)
{
    if (forNode == nullptr)
    {
        return;
    }

    Node* expr1 = forNode;
    Node* expr2 = getNextLine(expr1);
    Node* expr3 = getNextLine(expr2);
    Node* bodyBlock = getNextLine(expr3);

    executeAssignment(expr1);

    while (!isReturning && evaluateExpression(expr2->sibling))
    {
        executeBlock(bodyBlock);

        if (!isReturning)
        {
            executeAssignment(expr3);
        }
    }
}


void Interpreter::executePrintf(Node* printfNode)
{
    if (printfNode == nullptr) return;
    // The first sibling of the PRINTF node is the format string
    Node* current = printfNode->sibling;
    string formatString = "";
    // build the format string
    while (current != nullptr && (current->label == "STRING" || current->label == "ESCAPED_CHARACTER"))
    {
        if (current->label == "ESCAPED_CHARACTER")
        {
            // if it's an escaped character, decode it and add the resulting character to the format string
            formatString += decodeCharLiteral(current->text);
        }
        else
        {
            // if it's a string literal add its text to the format string
            formatString += current->text;
        }
        current = current->sibling;
    }
    // Walk through each character of the format string
    for (size_t i = 0; i < formatString.length(); i++)
    {
        // If we encounter a '%' character we check the next character to determine the type of the argument to print.
        if (formatString[i] == '%' && i + 1 < formatString.length())
        {
            char type = formatString[i + 1];
            if (type == 'd')
            {
                if (current != nullptr)
                {
                    if (current->label == "IDENTIFIER") {
                        SymbolNode* sym = lookupSymbol(current->text);
                        out << getIntValue(sym);
                    } else if (current->label == "INTEGER") {
                        out << stoi(current->text);
                    }
                    current = current->sibling;
                }
                i++;
            }
            else if (type == 's')
            {
                if (current != nullptr)
                {
                    if (current->label == "IDENTIFIER")
                    {
                        SymbolNode* sym = lookupSymbol(current->text);
                        if (sym->datatype == "char" && sym->isArray)
                        {
                            for (int k = 0; k < sym->arraySize; k++)
                            {
                                char c = getCharArrayValue(sym, k);
                                if (c == '\0') break;
                                out << c;
                            }
                        }
                    }
                    current = current->sibling;
                }
                i++;
            }
            else
            {
                out << formatString[i];
            }
        }
        else
        {
            out << formatString[i];
        }
    }
}
