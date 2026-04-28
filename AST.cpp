#include "AST.h"
#include <vector>
#include <stack>

using namespace std;

AST::AST()
    : concreteSyntaxTree(nullptr),
      abstractSyntaxTree(nullptr),
      lastAstElement(nullptr),
      buildError(false),
      buildErrorMessage(""),
      buildErrorLine(0),
      nextLinkIsChild(false)
{
}

bool isReservedWord(const string &word)
{
    return word == "function" ||
           word == "procedure" ||
           word == "if" ||
           word == "else" ||
           word == "while" ||
           word == "for" ||
           word == "return" ||
           word == "int" ||
           word == "char" ||
           word == "bool" ||
           word == "void" ||
           word == "float" ||
           word == "double" ||
           word == "printf" ||
           word == "TRUE" ||
           word == "FALSE";
}

bool isDeclarationStart(Node *n)
{
    if (n == nullptr)
        return false;
    if (n->label != "IDENTIFIER")
        return false;

    return n->text == "int" ||
           n->text == "char" ||
           n->text == "bool" ||
           n->text == "void" ||
           n->text == "float" ||
           n->text == "double";
}

bool isFunctionOrProcedureStart(Node *n)
{
    if (n == nullptr)
        return false;
    if (n->label != "IDENTIFIER")
        return false;

    return n->text == "function" || n->text == "procedure";
}

bool isCallStart(Node *n)
{
    if (n == nullptr)
        return false;
    if (n->label != "IDENTIFIER")
        return false;
    if (isReservedWord(n->text))
        return false;
    if (n->sibling == nullptr)
        return false;

    return n->sibling->text == "(";
}

bool isAssignmentStart(Node *n)
{
    if (n == nullptr)
        return false;
    if (n->label != "IDENTIFIER")
        return false;
    if (isReservedWord(n->text))
        return false;

    Node *cur = n->sibling;

    while (cur != nullptr)
    {
        if (cur->text == ";")
        {
            return false;
        }
        if (cur->text == "=")
        {
            return true;
        }
        if (cur->text == "(")
        {
            return false;
        }
        cur = cur->sibling;
    }

    return false;
}

bool isPrintfStart(Node *n)
{
    return n != nullptr && n->label == "IDENTIFIER" && n->text == "printf";
}

bool isReturnStart(Node *n)
{
    return n != nullptr && n->label == "IDENTIFIER" && n->text == "return";
}

bool isIfStart(Node *n)
{
    return n != nullptr && n->label == "IDENTIFIER" && n->text == "if";
}

bool isElse(Node *n)
{
    return n != nullptr && n->label == "IDENTIFIER" && n->text == "else";
}

bool isWhileStart(Node *n)
{
    return n != nullptr && n->label == "IDENTIFIER" && n->text == "while";
}

bool isForStart(Node *n)
{
    return n != nullptr && n->label == "IDENTIFIER" && n->text == "for";
}

bool isLeftBrace(Node *n)
{
    return n != nullptr && n->text == "{";
}

bool isRightBrace(Node *n)
{
    return n != nullptr && n->text == "}";
}

bool isSemicolon(Node *n)
{
    return n != nullptr && n->text == ";";
}

AST::~AST()
{
    destroyTree(abstractSyntaxTree);
}

Node *AST::getTree() const
{
    return abstractSyntaxTree;
}

bool AST::hasError() const
{
    return buildError;
}

string AST::getErrorMessage() const
{
    return buildErrorMessage;
}

int AST::getErrorLine() const
{
    return buildErrorLine;
}

void AST::printTree(ostream &out) const
{
    if (abstractSyntaxTree == nullptr)
    {
        out << "(empty tree)";
        return;
    }

    printTreeLines(abstractSyntaxTree, out);
}

void AST::printTreeLines(const Node *start, ostream &out) const
{
    if (start == nullptr)
    {
        return;
    }

    const Node *cur = start;
    const Node *nextLine = nullptr;

    while (cur != nullptr)
    {
        string text = cur->text.empty() ? cur->label : cur->text;
        out << text << "    ";

        if (cur->child != nullptr)
        {
            nextLine = cur->child;
        }

        cur = cur->sibling;
    }

    out << '\n';
    printTreeLines(nextLine, out);
}

void AST::destroyTree(Node *n)
{
    if (n == nullptr)
    {
        return;
    }

    destroyTree(n->child);
    destroyTree(n->sibling);
    delete n;
}

Node *AST::createAstNode(const string &label, const string &text, int line)
{
    return new Node(label, text, line);
}

void AST::setNextLinkToSibling()
{
    nextLinkIsChild = false;
}

void AST::setNextLinkToChild()
{
    nextLinkIsChild = true;
}

Node *AST::addElementToAbstractSyntaxTree(const string &label, const string &text, int line)
{
    Node *newAstElement = createAstNode(label, text, line);

    if (newAstElement == nullptr)
    {
        return nullptr;
    }

    if (abstractSyntaxTree == nullptr)
    {
        abstractSyntaxTree = newAstElement;
        lastAstElement = newAstElement;
        nextLinkIsChild = false;
        return newAstElement;
    }

    if (nextLinkIsChild)
    {
        lastAstElement->child = newAstElement;
        lastAstElement = newAstElement;
        nextLinkIsChild = false;
    }
    else
    {
        lastAstElement->sibling = newAstElement;
        lastAstElement = newAstElement;
    }

    return newAstElement;
}

Node *nextCstNode(Node *current)
{
    if (current == nullptr)
    {
        return nullptr;
    }

    if (current->sibling != nullptr)
    {
        return current->sibling;
    }

    if (current->child != nullptr)
    {
        return current->child;
    }

    return nullptr;
}

bool isOperator(const Node *n)
{
    if (n == nullptr)
        return false;

    return n->text == "!" ||
           n->text == "^" ||
           n->text == "*" ||
           n->text == "/" ||
           n->text == "%" ||
           n->text == "+" ||
           n->text == "-" ||
           n->text == "<" ||
           n->text == "<=" ||
           n->text == ">" ||
           n->text == ">=" ||
           n->text == "==" ||
           n->text == "!=" ||
           n->text == "&&" ||
           n->text == "||";
}

int precedence(const Node *n)
{
    if (n == nullptr)
        return -1;

    if (n->text == "!")
        return 6;
    if (n->text == "^")
        return 5;
    if (n->text == "*" || n->text == "/" || n->text == "%")
        return 4;
    if (n->text == "+" || n->text == "-")
        return 3;
    if (n->text == "<" || n->text == "<=" ||
        n->text == ">" || n->text == ">=" ||
        n->text == "==" || n->text == "!=")
        return 2;
    if (n->text == "&&")
        return 1;
    if (n->text == "||")
        return 0;

    return -1;
}

void AST::shuntingYard(const vector<Node*>& list) {
    stack<Node*> stack;
    vector<Node*> output;

    for (size_t i = 0; i < list.size(); i++) {
        Node* curr = list[i];
        
        if (curr->text == "\"") { // string literal
            int literalLine = curr->line;
            output.push_back(curr);
            i++;
            string literal = "";

            while (i < list.size() && list[i]->text != "\"") {
                literal += list[i]->text;
                i++;
            }
            Node* literalNode = createAstNode("STRING", literal, literalLine);
            output.push_back(literalNode);
            output.push_back(list[i]);
        }
        else if (curr->label == "IDENTIFIER" && (i + 1 < list.size()) && (list[i + 1]->text == "(")) { // function call - add function name and entire parenthesis group as one unit
                
                output.push_back(curr);
                i++;
                output.push_back(list[i]);
                int parenDepth = 1;
                i++;
                while (i < list.size() && parenDepth > 0) {
                    output.push_back(list[i]);

                    if (list[i]->text == "(") {
                        parenDepth++;
                    }
                    else if (list[i]->text == ")") {
                        parenDepth--;
                    }

                    i++;
                }
                i--;
            }

        // if single-quoted literal, treat whole thing like one operand group
        else if (curr->text == "'") {
            int literalLine = curr->line;
            output.push_back(curr); 
            i++;

            string literal = "";

            while (i < list.size() && list[i]->text != "'") {
                literal += list[i]->text;
                i++;
            }

            Node* literalNode = createAstNode("ESCAPED_CHARACTER", literal, literalLine);
            output.push_back(literalNode);
            output.push_back(list[i]);
        }
        // if operand, append to output
        
        else if (curr->label == "IDENTIFIER" ||
            curr->label == "INTEGER" ||
            curr->text == "TRUE" ||
            curr->text == "FALSE") {
            output.push_back(curr);
        }
        // if left parenthesis, stack
        else if (curr->text == "(") {
            stack.push(curr);
        }
        // if right parenthesis, pop stack until find left parenthesis
        else if (curr->text == ")") {
            while (!stack.empty() && stack.top()->text != "(") {
                output.push_back(stack.top());
                stack.pop();
            }

            if (!stack.empty()) {
                stack.pop(); // remove (
            }
        }
        // if operator, first remove any higher or equal precedence, then push to stack
        else if (isOperator(curr)) {
            while (!stack.empty() &&
                   stack.top()->text != "(" &&
                   ((precedence(curr) < precedence(stack.top())) ||
                    (precedence(curr) == precedence(stack.top()) && curr->text != "^"))) {
                output.push_back(stack.top());
                stack.pop();
            }

            stack.push(curr);
        }
    }

    // final stack flush
    while (!stack.empty()) {
        if (stack.top()->text != "(" && stack.top()->text != ")") {
            output.push_back(stack.top());
        }
        stack.pop();
    }

    // add postfix result directly into AST
    for (Node* curr : output) {
        addElementToAbstractSyntaxTree(curr->label, curr->text, curr->line);
    }
}

Node *AST::build(Node *cstRoot)
{
    concreteSyntaxTree = cstRoot;
    nextLinkIsChild = false;
    Node *current = concreteSyntaxTree;
    vector<Node *> infixTokens;
    enum states
    {
        START,
        FUNC_PROC,
        DECLARATION,
        PRINT,
        RETURN,
        IF,
        ELSE,
        WHILE,
        FOR,
        CALL,
        ASSIGNMENT
    };
    states state = START;
    destroyTree(abstractSyntaxTree);
    abstractSyntaxTree = nullptr;
    lastAstElement = nullptr;
    buildError = false;
    buildErrorMessage = "";
    buildErrorLine = 0;

    nextLinkIsChild = false;
    while (current != nullptr)
    {
        if (state == START)
        {
            if (isFunctionOrProcedureStart(current))
            {
                addElementToAbstractSyntaxTree("DECLARATION", "", current->line);
                setNextLinkToChild();
                current = nextCstNode(current);
                state = FUNC_PROC;
            }
            else if (isDeclarationStart(current))
            {
                addElementToAbstractSyntaxTree("DECLARATION", "", current->line);
                setNextLinkToChild();
                current = nextCstNode(current);
                state = DECLARATION;
            }
            else if (isPrintfStart(current))
            {
                addElementToAbstractSyntaxTree("PRINTF", "", current->line);
                current = nextCstNode(current);
                state = PRINT;
            }
            else if (isReturnStart(current))
            {
                addElementToAbstractSyntaxTree("RETURN", "", current->line);
                current = nextCstNode(current);
                state = RETURN;
            }
            else if (isIfStart(current))
            {
                addElementToAbstractSyntaxTree("IF", "", current->line);
                current = nextCstNode(current);
                state = IF;
            }
            else if (isElse(current))
            {
                addElementToAbstractSyntaxTree("ELSE", "", current->line);
                current = nextCstNode(current);
                setNextLinkToChild();
                // stay in START state since else statement is just else followed by block
            }
            else if (isWhileStart(current))
            {
                addElementToAbstractSyntaxTree("WHILE", "", current->line);
                current = nextCstNode(current);
                state = WHILE;
            }
            else if (isForStart(current))
            {
                addElementToAbstractSyntaxTree("FOR", "", current->line);
                current = nextCstNode(current);
                state = FOR;
            }
            else if (isCallStart(current))
            {
                addElementToAbstractSyntaxTree("CALL", "", current->line);
                state = CALL;
            }
            else if (isAssignmentStart(current))
            {
                addElementToAbstractSyntaxTree("ASSIGNMENT", "", current->line);
                state = ASSIGNMENT;
            }
            else if (isLeftBrace(current))
            {
                addElementToAbstractSyntaxTree("BEGIN BLOCK", "", current->line);
                setNextLinkToChild();
                current = nextCstNode(current);
            }
            else if (isRightBrace(current))
            {
                addElementToAbstractSyntaxTree("END BLOCK", "", current->line);
                setNextLinkToChild();
                current = nextCstNode(current);
            }
            else
            {
                current = nextCstNode(current);
            }
        }
        else if (state == FUNC_PROC)
        { // inside function/procedure
            if (isLeftBrace(current))
            {
                state = START;
            }
            else
            {
                current = nextCstNode(current);
            }
        }
        else if (state == DECLARATION)
        { // inside declaration
            if (isSemicolon(current))
            {
                state = START;
                current = nextCstNode(current);
            }
            else if (current->text == ",")
            { // handle multiple declarations in one line
                addElementToAbstractSyntaxTree("DECLARATION", "", current->line);
                setNextLinkToChild();
                current = nextCstNode(current);
            }
            else if (isLeftBrace(current))
            {
                state = START;
            }
            else
            {
                current = nextCstNode(current);
            }
        }
        else if (state == PRINT)
        { // inside printf
            if (isSemicolon(current))
            {
                state = START;
                setNextLinkToChild();
                current = nextCstNode(current);
            }
            else if (current->text == "(" || current->text == ")" || current->text == ",")
            {
                current = nextCstNode(current);
            }
            else if (current->text == "\"")
            {
                current = nextCstNode(current);
                while (current != nullptr && current->text != "\"")
                {
                    addElementToAbstractSyntaxTree(current->label, current->text, current->line);
                    current = nextCstNode(current);
                }
                if (current != nullptr && current->text == "\"")
                {
                    current = nextCstNode(current);
                }
            }
            else
            {
                addElementToAbstractSyntaxTree(current->label, current->text, current->line);
                current = nextCstNode(current);
            }
        }
        else if (state == RETURN)
        { // inside return
            if (isSemicolon(current))
            {
                state = START;
                setNextLinkToChild();
            }
            else
            {
                addElementToAbstractSyntaxTree(current->label, current->text, current->line);
            }
            current = nextCstNode(current);
        }
        else if (state == IF)
        { // inside if
            if (isLeftBrace(current))
            {
                state = START;
                shuntingYard(infixTokens);
                setNextLinkToChild();
                infixTokens.clear();
            }
            else
            {
                infixTokens.push_back(current);
                current = nextCstNode(current);
            }
        }
        else if (state == WHILE)
        { // inside while
            if (isLeftBrace(current))
            {
                state = START;
                shuntingYard(infixTokens);
                setNextLinkToChild();
                infixTokens.clear();
            }
            else
            {
                infixTokens.push_back(current);
                current = nextCstNode(current);
            }
        }
        else if (state == FOR)
        { // inside for
            // Will make latter
            current = nextCstNode(current);
            state = START;
        }
        else if (state == CALL)
        { // inside call
            if (isSemicolon(current))
            {
                state = START;
                setNextLinkToChild();
            }
            else
            {
                addElementToAbstractSyntaxTree(current->label, current->text, current->line);
            }
            current = nextCstNode(current);
        }
        else if (state == ASSIGNMENT)
        { // inside assignment
            if (isSemicolon(current))
            {
                state = START;

                vector<Node *> leftSide;
                vector<Node *> rightSide;
                bool seenEquals = false;

                for (Node *tok : infixTokens)
                {
                    if (tok->text == "=")
                    {
                        seenEquals = true;
                    }
                    else if (!seenEquals)
                    {
                        leftSide.push_back(tok);
                    }
                    else
                    {
                        rightSide.push_back(tok);
                    }
                }

                for (Node *tok : leftSide)
                {
                    addElementToAbstractSyntaxTree(tok->label, tok->text, tok->line);
                }

                shuntingYard(rightSide);
                addElementToAbstractSyntaxTree("ASSIGN", "=", current->line);

                infixTokens.clear();
                setNextLinkToChild();
            }
            else
            {
                infixTokens.push_back(current);
            }

            current = nextCstNode(current);
        }
        else
        {
            break;
        }
    }
    return abstractSyntaxTree;
}