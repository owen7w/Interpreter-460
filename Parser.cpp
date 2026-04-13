#include "Parser.h"
#include <queue>

Parser::Parser(const vector<Token>& toks)
    : tokens(toks),
      currentTokenIndex(0),
      concreteSyntaxTree(nullptr),
      lastCstElement(nullptr),
      newCstElement(nullptr),
      previousTokenNode(nullptr),
      parseError(false),
      parseErrorMessage(""),
      parseErrorLine(0),
      nextLinkIsChild(false),
      state(0),
      numLeftParenthesis(0),
      numRightParenthesis(0),
      numLeftBraces(0),
      numRightBraces(0),
      numLeftBrackets(0),
      numRightBrackets(0) {
}

Parser::~Parser() {
    destroyTree(concreteSyntaxTree);
}

const Token& Parser::currentToken() const {
    static Token eofToken = {"EOF", "EOF", 1};

    if (currentTokenIndex >= tokens.size()) {
        if (!tokens.empty()) {
            eofToken.line = tokens.back().line;
        }
        return eofToken;
    }

    return tokens[currentTokenIndex];
}

const Token& Parser::peek(int offset) const {
    static Token eofToken = {"EOF", "EOF", 1};

    size_t index = currentTokenIndex + static_cast<size_t>(offset);
    if (index >= tokens.size()) {
        if (!tokens.empty()) {
            eofToken.line = tokens.back().line;
        }
        return eofToken;
    }

    return tokens[index];
}

bool Parser::atEnd() const {
    return currentTokenIndex >= tokens.size() ||
           currentToken().type == "EOF";
}

void Parser::advance() {
    if (!atEnd()) {
        currentTokenIndex++;
    }
}

bool Parser::checkType(const string& type, int offset) const {
    return peek(offset).type == type;
}

bool Parser::checkValue(const string& value, int offset) const {
    return peek(offset).value == value;
}

bool Parser::isTypeWord(const Token& t) const {
    if (t.type != "IDENTIFIER") {
        return false;
    }

    return t.value == "int" ||
           t.value == "char" ||
           t.value == "bool" ||
           t.value == "void" ||
           t.value == "float" ||
           t.value == "double";
}

bool Parser::isReservedWord(const string& word) const {
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
           //word == "string" || sting is allowed i guess based on test 4
           word == "printf" ||
           word == "TRUE" ||
           word == "FALSE";
}

Node* Parser::createNode(const string& label, const string& text, int line) {
    return new Node(label, text, line);
}

Node* Parser::createTokenNode(const Token& t) {
    return createNode(t.type, t.value, t.line);
}

void Parser::setNextLinkToSibling() {
    nextLinkIsChild = false;
}


void Parser::setNextLinkToChild() {
    nextLinkIsChild = true;
}

Node* Parser::addElementToConcreteSyntaxTree() {
    if (atEnd()) {
        return nullptr;
    }

    newCstElement = createTokenNode(currentToken());

    if (newCstElement == nullptr) {
        fail(currentToken().line, "CSTNode creation failed somehow?");
        return nullptr;
    }

    if (concreteSyntaxTree == nullptr) {
        concreteSyntaxTree = newCstElement;
        lastCstElement = newCstElement;
        previousTokenNode = nullptr;
        nextLinkIsChild = false;
        return newCstElement;
    }

    previousTokenNode = lastCstElement;

    if (nextLinkIsChild) {
        lastCstElement->child = newCstElement;
        lastCstElement = newCstElement;
        nextLinkIsChild = false;   
    } else {
        lastCstElement->sibling = newCstElement;
        lastCstElement = newCstElement;
    }
    return newCstElement;
}


void Parser::fail(int line, const string& message) {
    parseError = true;
    parseErrorLine = line;
    parseErrorMessage = "Syntax error on line " + to_string(line) + ": " + message;
}


void Parser::printTree(ostream& out) const {
    if (concreteSyntaxTree == nullptr) {
        out << "(empty tree)";
        return;
    }

    printTreeLines(concreteSyntaxTree, out);
}

void Parser::printTreeLines(const Node* start, ostream& out) const {
    if (start == nullptr) {
        return;
    }


    string firstText = start->text.empty() ? start->label : start->text;


    const Node* cur = start;
    const Node* nextLine = nullptr;
    string lastText;

    while (cur != nullptr) {
        string text = cur->text.empty() ? cur->label : cur->text;
        lastText = text;

        out << text << '\t';  

        if (cur->child != nullptr) {
            nextLine = cur->child;
        }

        cur = cur->sibling;
    }

    out << '\n';
    printTreeLines(nextLine, out);
}







Node* Parser::parse() {
    destroyTree(concreteSyntaxTree);

    concreteSyntaxTree = nullptr;
    lastCstElement = nullptr;
    newCstElement = nullptr;
    previousTokenNode = nullptr;

    parseError = false;
    parseErrorMessage = "";
    parseErrorLine = 0;

    currentTokenIndex = 0;
    state = 0;
    nextLinkIsChild = false;

    numLeftParenthesis = 0;
    numRightParenthesis = 0;
    numLeftBraces = 0;
    numRightBraces = 0;
    numLeftBrackets = 0;
    numRightBrackets = 0;
    int statementParenthesisDepth = 0;
    int statementBracketDepth = 0;
    while (!parseError ) {
        switch (state) {

            case 0: // top level, expect function/procedure declarations or global variable declarations
                if (atEnd()) {
                    return concreteSyntaxTree;
                }

                if (checkType("IDENTIFIER")) {
                    if (checkValue("function")) {
                        addElementToConcreteSyntaxTree();
                        advance();
                        state = 1000;
                    }
                    else if (checkValue("procedure")) {
                        addElementToConcreteSyntaxTree();
                        advance();
                        state = 2000;
                    }
                    else if (isTypeWord(currentToken())) {
                        addElementToConcreteSyntaxTree();
                        advance();
                        state = 100;
                    }
                    else {
                        fail(currentToken().line,
                             "unexpected token '" + currentToken().value +
                             "' at top level. ");
                    }
                }
                else {
                    fail(currentToken().line,
                         "unexpected token '" + currentToken().value +
                         "' at top level.");
                }
                break;

            case 100: // global variable declaration after datatype
                if (!checkType("IDENTIFIER")) {
                    fail(currentToken().line, "expected identifier after datatype.");
                }
                else if (isReservedWord(currentToken().value)) {
                    fail(currentToken().line,
                        "reserved word '" + currentToken().value +
                        "' cannot be used for the name of a variable.");
                }
                else {
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();   
                    advance();
                    state = 101;
                }
                break;
            case 101: // after identifier in global variable declaration, expect array decl or more variables or semicolon
                if (checkType("L_BRACKET")) {
                    numLeftBrackets++;
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();   
                    advance();
                    state = 102;
                }
                else if (checkType("COMMA")) {
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();   
                    advance();
                    state = 100;   
                }
                else if (checkType("SEMICOLON")) {
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();  
                    advance();

                    setNextLinkToChild();   
                    state = 0;
                }
                else {
                    fail(currentToken().line,
                        " global declaration is wrong someway");
                }
                break;
            case 102: // after [ in array declaration expect size
                if (!checkType("INTEGER")) {
                    fail(currentToken().line,
                        "array declaration size must be a positive integer");
                }
                else {
                    string s = currentToken().value;

                    if (!s.empty() && s[0] == '-') {
                        fail(currentToken().line,
                            "array declaration size must be a positive integer");
                    }
                    else {
                        if (!s.empty() && s[0] == '+') {
                            s = s.substr(1);
                        }

                        if (s == "0") {
                            fail(currentToken().line,
                                "array declaration size must be a positive integer");
                        }
                        else {
                            setNextLinkToSibling();
                            addElementToConcreteSyntaxTree();   
                            advance();
                            state = 103;
                        }
                    }
                }
                break;
            case 103:
                if (!checkType("R_BRACKET")) {
                    fail(currentToken().line, "expected ].");
                }
                else {
                    numRightBrackets++;
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();  
                    advance();
                    state = 101;   
                }
                break;





            case 1000: // after function, expect return type
                if (!checkType("IDENTIFIER") || !isTypeWord(currentToken())) {
                    fail(currentToken().line,
                        "expected return datatype after function");
                }
                else {
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();   
                    advance();
                    state = 1001;
                }
                break;

            case 1001: // after function return type, expect function name
                if (!checkType("IDENTIFIER")) {
                    fail(currentToken().line, "expected function name.");
                }
                else if (isReservedWord(currentToken().value)) {
                    fail(currentToken().line,
                        "reserved word '" + currentToken().value +
                        "' cannot be used for the name of a function.");
                }
                else {
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();   
                    advance();
                    state = 1002;
                }
                break;

            case 1002: // after function name, expect left parenthesis for parameter list
                if (!checkType("L_PAREN")) {
                    fail(currentToken().line, "expected '(' after function name.");
                }
                else {
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();   // (
                    advance();
                    state = 1003;
                }
                break;

            case 1003: // in function parameter list, expect parameter type or right parenthesis
                if (checkType("R_PAREN")) {
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();   
                    advance();
                    setNextLinkToChild();
                    state = 1008;
                }
                else if (checkType("IDENTIFIER") && isTypeWord(currentToken())) {
                    if (currentToken().value == "void" && checkType("R_PAREN", 1)) { // void is allowed
                        setNextLinkToSibling();
                        addElementToConcreteSyntaxTree();   
                        advance();

                        setNextLinkToSibling();
                        addElementToConcreteSyntaxTree();  
                        advance();

                        setNextLinkToChild();
                        state = 1008;
                    }
                    else {
                        setNextLinkToSibling();
                        addElementToConcreteSyntaxTree();   
                        advance();
                        state = 1004;
                    }
                }
                else {
                    fail(currentToken().line,
                        "expected datatype or ')' in function parameter list.");
                }
                break;

            case 1004: // after parameter type expect parameter name
                if (!checkType("IDENTIFIER")) {
                    fail(currentToken().line, "expected parameter name.");
                }
                else if (isReservedWord(currentToken().value)) {
                    fail(currentToken().line,
                        "reserved word '" + currentToken().value +
                        "' cannot be used for the name of a variable.");
                }
                else {
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();   
                    advance();
                    state = 1005;
                }
                break;

            case 1005: // after parameter name
                if (checkType("L_BRACKET")) {
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();   
                    advance();
                    state = 1006;
                }
                else if (checkType("COMMA")) {
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();   
                    advance();
                    state = 1003;
                }
                else if (checkType("R_PAREN")) {
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();  
                    advance();
                    setNextLinkToChild();
                    state = 1008;
                }
                else {
                    fail(currentToken().line,
                        " parameter declaration is wrong someway");
                }
                break;

            case 1006: // array size
                if (!checkType("INTEGER")) {
                    fail(currentToken().line,
                        "array declaration size must be a positive integer");
                }
                else {
                    string s = currentToken().value;

                    if (!s.empty() && s[0] == '-') {
                        fail(currentToken().line,
                            "array declaration size must be a positive integer");
                    }
                    else {
                        if (!s.empty() && s[0] == '+') {
                            s = s.substr(1);
                        }
                        if (s == "0") {
                            fail(currentToken().line,
                                "array declaration size must be a positive integer");
                        }
                        else {
                            setNextLinkToSibling();
                            addElementToConcreteSyntaxTree();
                            advance();
                            state = 1007;
                        }
                    }
                }
                break;

            case 1007: // array
                if (!checkType("R_BRACKET")) {
                    fail(currentToken().line, "expected ]");
                }
                else {
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();   
                    advance();
                    state = 1005;
                }
                break;

            case 1008:
                if (!checkType("L_BRACE")) {
                    fail(currentToken().line,
                        "expected '{' to start function body.");
                }
                else {
                    numLeftBraces = 1;
                    numRightBraces = 0;

                    addElementToConcreteSyntaxTree();   
                    advance();

                    setNextLinkToChild();
                    state = 3000;
                }
                break;

                /*
                * The states for procedure declarations are mostly the same as function declarations
                * Keep it separate just in case we check return type for functions but not procedures, or something like that in the future
                * also better error messages so we know we're parsing a procedure vs a function
                */















            case 2000:
                if (!checkType("IDENTIFIER")) {
                    fail(currentToken().line, "expected procedure name.");
                }
                else if (isReservedWord(currentToken().value)) {
                    fail(currentToken().line,
                        "reserved word '" + currentToken().value +
                        "' cannot be used for the name of a procedure.");
                }
                else {
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();  
                    advance();
                    state = 2001;
                }
                break;



            case 2001:
                if (!checkType("L_PAREN")) {
                    fail(currentToken().line,
                        "expected ( after procedure name.");
                }
                else {
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();  
                    advance();
                    state = 2002;
                }
                break;

            case 2002: // void is allowed here so it a special case based on test 8
                if (checkType("R_PAREN")) {
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();  
                    advance();
                    setNextLinkToChild();
                    state = 2007;
                }
                else if (checkType("IDENTIFIER") && isTypeWord(currentToken())) {
                    if (currentToken().value == "void" && checkType("R_PAREN", 1)) {
                        setNextLinkToSibling();
                        addElementToConcreteSyntaxTree();   
                        advance();

                        setNextLinkToSibling();
                        addElementToConcreteSyntaxTree();   
                        advance();

                        setNextLinkToChild();
                        state = 2007;
                    }
                    else {
                        setNextLinkToSibling();
                        addElementToConcreteSyntaxTree();  
                        advance();
                        state = 2003;
                    }
                }
                else {
                    fail(currentToken().line,
                        "parameter list issue in procedure declaration.");
                }
                break;

            case 2003:
                if (!checkType("IDENTIFIER")) {
                    fail(currentToken().line, "expected parameter name.");
                }
                else if (isReservedWord(currentToken().value)) {
                    fail(currentToken().line,
                        "reserved word '" + currentToken().value +
                        "' cannot be used for the name of a variable.");
                }
                else {
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();   // parameter name
                    advance();
                    state = 2004;
                }
                break;

            case 2004:
                if (checkType("L_BRACKET")) {
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();   
                    advance();
                    state = 2005;
                }
                else if (checkType("COMMA")) {
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();  
                    advance();
                    state = 2002;
                }
                else if (checkType("R_PAREN")) {
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();   
                    advance();
                    setNextLinkToChild();
                    state = 2007;
                }
                else {
                    fail(currentToken().line,
                        "parameter is wrong someway in procedure declaration.");
                }
                break;

            case 2005:
                if (!checkType("INTEGER")) {
                    fail(currentToken().line,
                        "array declaration size must be a positive integer");
                }
                else {
                    string s = currentToken().value;

                    if (!s.empty() && s[0] == '-') {
                        fail(currentToken().line,
                            "array declaration size must be a positive integer");
                    }
                    else {
                        if (!s.empty() && s[0] == '+') {
                            s = s.substr(1);
                        }

                        if (s == "0") {
                            fail(currentToken().line,
                                "array declaration size must be a positive integer");
                        }
                        else {
                            setNextLinkToSibling();
                            addElementToConcreteSyntaxTree();  
                            advance();
                            state = 2006;
                        }
                    }
                }
                break;

            case 2006:
                if (!checkType("R_BRACKET")) {
                    fail(currentToken().line, "expected ]");
                }
                else {
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();   
                    advance();
                    state = 2004;
                }
                break;

            case 2007:
                if (!checkType("L_BRACE")) {
                    fail(currentToken().line,
                        "expected { to start procedure body.");
                }
                else {
                    numLeftBraces = 1;
                    numRightBraces = 0;

                    addElementToConcreteSyntaxTree();   
                    advance();

                    setNextLinkToChild();   
                    state = 3000;
                }
                break;




















            /*
            * All the states for parsing statements inside function/procedure bodies are in this 3000s range
            */



            case 3000: // general statement parsing state for inside something
                if (atEnd()) {
                    fail(currentToken().line,
                        "unexpected end of file inside function/procedure body.");
                }
                else if (checkType("L_BRACE")) {
                    numLeftBraces++;
                    addElementToConcreteSyntaxTree();  
                    advance();
                    setNextLinkToChild();   
                    state = 3000;
                }
                else if (checkType("R_BRACE")) {
                    numRightBraces++;
                    addElementToConcreteSyntaxTree();   
                    advance();
                    setNextLinkToChild();

                    if (numLeftBraces == numRightBraces) {
                        state = 0;
                    } else if (numRightBraces < numLeftBraces) {
                        state = 3000;
                    } else {
                        fail(previousTokenNode != nullptr ? previousTokenNode->line : 0,
                            "unbalanced curly braces.");
                    }
                }
                else if (checkType("IDENTIFIER") && isTypeWord(currentToken())) {
                    
                    addElementToConcreteSyntaxTree();   
                    advance();
                    state = 3100;
                }
                else if (checkType("IDENTIFIER") && checkValue("return")) {
                    statementParenthesisDepth = 0;
                    statementBracketDepth = 0;
                    addElementToConcreteSyntaxTree();   
                    advance();
                    state = 3200;
                }
                else if (checkType("IDENTIFIER") && checkValue("if")) {
                    statementParenthesisDepth = 0;
                    statementBracketDepth = 0;
                    addElementToConcreteSyntaxTree();   
                    advance();
                    state = 3300;
                }
                else if (checkType("IDENTIFIER") && checkValue("else")) { // else is weird   
                    addElementToConcreteSyntaxTree();   
                    advance();
                    state = 3310;
                }
                else if (checkType("IDENTIFIER") && checkValue("while")) {
                    statementParenthesisDepth = 0;
                    statementBracketDepth = 0;
                    addElementToConcreteSyntaxTree();  
                    advance();
                    state = 3500;
                }
                else if (checkType("IDENTIFIER") && checkValue("for")) {
                    statementParenthesisDepth = 0;
                    statementBracketDepth = 0;
                    addElementToConcreteSyntaxTree();   
                    advance();
                    state = 3600;
                }
                else {
                    statementParenthesisDepth = 0;
                    statementBracketDepth = 0;
                    addElementToConcreteSyntaxTree();   
                    advance();
                    state = 3400;
                }
                break;




            case 3100: // after datatype in a local variable declaration expect variable name
                if (!checkType("IDENTIFIER")) {
                    fail(currentToken().line, "expected identifier after datatype.");
                }
                else if (isReservedWord(currentToken().value)) {
                    fail(currentToken().line,
                        "reserved word '" + currentToken().value +
                        "' cannot be used for the name of a variable.");
                }
                else {
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();   
                    advance();
                    state = 3101;
                }
                break;   

            case 3101: // expect array decl or more variables or semicolon
                if (checkType("L_BRACKET")) {
                    numLeftBrackets++;
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();   
                    advance();
                    state = 3102;
                }
                else if (checkType("COMMA")) {
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();   
                    advance();
                    state = 3100;   
                }
                else if (checkType("SEMICOLON")) {
                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();  
                    advance();

                    setNextLinkToChild();   
                    state = 3000;
                }
                else {
                    fail(currentToken().line,
                        "local declaration is wrong someway");
                }
                break;



            case 3102: // after [ in array declaration expect size
                if (!checkType("INTEGER")) {
                    fail(currentToken().line,
                        "array declaration size must be a positive integer");
                }
                else {
                    string s = currentToken().value;

                    if (!s.empty() && s[0] == '-') {
                        fail(currentToken().line,
                            "array declaration size must be a positive integer");
                    }
                    else {
                        if (!s.empty() && s[0] == '+') {
                            s = s.substr(1);
                        }

                        if (s == "0") {
                            fail(currentToken().line,
                                "array declaration size must be a positive integer");
                        }
                        else {
                            setNextLinkToSibling();
                            addElementToConcreteSyntaxTree();   
                            advance();
                            state = 3103;
                        }
                    }
                }
                break;
            

            case 3103: // after array size in array declaration expect ]
                if (!checkType("R_BRACKET")) {
                    fail(currentToken().line, "expected ]");
                }
                else {
                    numRightBrackets++;

                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();  
                    advance();
                    state = 3101;
                }
                break;
            

                // later might add a flag or something for procedures that return void so we know not to expect a return statement 
            case 3200: // return statement, expect anything that could be in a return statement 
                if (atEnd()) {
                    fail(currentToken().line,
                        "unexpected end of file in return statement.");
                }
                else {
                    Token t = currentToken();

                    if (t.type == "L_PAREN") {
                        statementParenthesisDepth++;
                    }
                    else if (t.type == "R_PAREN") {
                        statementParenthesisDepth--;
                        if (statementParenthesisDepth < 0) {
                            fail(t.line, "unbalanced parenthesis in return statement.");
                            break;
                        }
                    }
                    else if (t.type == "L_BRACKET") {
                        statementBracketDepth++;
                    }
                    else if (t.type == "R_BRACKET") {
                        statementBracketDepth--;
                        if (statementBracketDepth < 0) {
                            fail(t.line, "unbalanced bracket in return statement.");
                            break;
                        }
                    }

                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();
                    advance();

                    if (t.type == "SEMICOLON" &&
                        statementParenthesisDepth == 0 &&
                        statementBracketDepth == 0) {
                        setNextLinkToChild();
                        state = 3000;
                    }
                }
                break;


            
            case 3300: // if statement expect anything that could be in the condition of the
                if (atEnd()) {
                    fail(currentToken().line,
                        "unexpected end of file in if statement.");
                }
                else {
                    Token t = currentToken();

                    if (t.type == "L_PAREN") {
                        statementParenthesisDepth++;
                    }
                    else if (t.type == "R_PAREN") {
                        statementParenthesisDepth--;
                        if (statementParenthesisDepth < 0) {
                            fail(t.line, "unbalanced parenthesis in if statement.");
                            break;
                        }
                    }
                    else if (t.type == "L_BRACKET") {
                        statementBracketDepth++;
                    }
                    else if (t.type == "R_BRACKET") {
                        statementBracketDepth--;
                        if (statementBracketDepth < 0) {
                            fail(t.line, "unbalanced bracket in if statement.");
                            break;
                        }
                    }

                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();
                    advance();

                    if (t.type == "R_PAREN" &&
                        statementParenthesisDepth == 0 &&
                        statementBracketDepth == 0) {
                        setNextLinkToChild();   
                        state = 3000;
                    }
                }
                break;

            case 3310: //else 
                setNextLinkToChild();   
                state = 3000;
                break;




            case 3400: // general statment parse till we hit a semicolon at parenthesis/bracket depth 0
                if (atEnd()) {
                    fail(currentToken().line,
                        "unexpected end of file in statement.");
                }
                else {
                    Token t = currentToken();

                    if (t.type == "L_PAREN") {
                        statementParenthesisDepth++;
                    }
                    else if (t.type == "R_PAREN") {
                        statementParenthesisDepth--;
                        if (statementParenthesisDepth < 0) {
                            fail(t.line, "unbalanced parenthesis in statement.");
                            break;
                        }
                    }
                    else if (t.type == "L_BRACKET") {
                        statementBracketDepth++;
                    }
                    else if (t.type == "R_BRACKET") {
                        statementBracketDepth--;
                        if (statementBracketDepth < 0) {
                            fail(t.line, "unbalanced bracket in statement.");
                            break;
                        }
                    }

                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();
                    advance();

                    if (t.type == "SEMICOLON" &&
                        statementParenthesisDepth == 0 &&
                        statementBracketDepth == 0) {
                        setNextLinkToChild();
                        state = 3000;
                    }
                }
                break;


            case 3500: // while stament expect anything that could be condition
                if (atEnd()) {
                    fail(currentToken().line,
                        "unexpected end of file in while statement.");
                }
                else {
                    Token t = currentToken();

                    if (t.type == "L_PAREN") {
                        statementParenthesisDepth++;
                    }
                    else if (t.type == "R_PAREN") {
                        statementParenthesisDepth--;
                        if (statementParenthesisDepth < 0) {
                            fail(t.line, "unbalanced parenthesis in while statement.");
                            break;
                        }
                    }
                    else if (t.type == "L_BRACKET") {
                        statementBracketDepth++;
                    }
                    else if (t.type == "R_BRACKET") {
                        statementBracketDepth--;
                        if (statementBracketDepth < 0) {
                            fail(t.line, "unbalanced bracket in while statement.");
                            break;
                        }
                    }

                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();
                    advance();

                    if (t.type == "R_PAREN" &&
                        statementParenthesisDepth == 0 &&
                        statementBracketDepth == 0) {
                        setNextLinkToChild();   
                        state = 3000;
                    }
                }
                break;


            case 3600: // for stament expect anything that could be condition
                if (atEnd()) {
                    fail(currentToken().line,
                        "unexpected end of file in for statement.");
                }
                else {
                    Token t = currentToken();

                    if (t.type == "L_PAREN") {
                        statementParenthesisDepth++;
                    }
                    else if (t.type == "R_PAREN") {
                        statementParenthesisDepth--;
                        if (statementParenthesisDepth < 0) {
                            fail(t.line, "unbalanced parenthesis in for statement.");
                            break;
                        }
                    }
                    else if (t.type == "L_BRACKET") {
                        statementBracketDepth++;
                    }
                    else if (t.type == "R_BRACKET") {
                        statementBracketDepth--;
                        if (statementBracketDepth < 0) {
                            fail(t.line, "unbalanced bracket in for statement.");
                            break;
                        }
                    }

                    setNextLinkToSibling();
                    addElementToConcreteSyntaxTree();
                    advance();

                    if (t.type == "R_PAREN" &&
                        statementParenthesisDepth == 0 &&
                        statementBracketDepth == 0) {
                        setNextLinkToChild();   
                        state = 3000;
                    }
                }
                break;




            default: // should never get here hopefully
                fail(currentToken().line, "parser state error.");
                break;
        }
    }

    if (parseError) {
        destroyTree(concreteSyntaxTree);
        concreteSyntaxTree = nullptr;
        return nullptr;
    }

    if (state != 0) {
        fail(currentToken().line, "unexpected end of file.");
        destroyTree(concreteSyntaxTree);
        concreteSyntaxTree = nullptr;
        return nullptr;
    }
    return concreteSyntaxTree;
}







bool Parser::hasError() const {
    return parseError;
}

string Parser::getErrorMessage() const {
    return parseErrorMessage;
}

int Parser::getErrorLine() const {
    return parseErrorLine;
}

Node* Parser::getTree() const {
    return concreteSyntaxTree;
}

void Parser::destroyTree(Node* n) {
    if (n == nullptr) {
        return;
    }
    destroyTree(n->child);
    destroyTree(n->sibling);
    delete n;
}






