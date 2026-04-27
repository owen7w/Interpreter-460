#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include <iostream>
#include "Tokenization.h"
#include "Node.h"

using namespace std;

class Parser {
private:
    vector<Token> tokens;
    size_t currentTokenIndex;

    Node* concreteSyntaxTree;
    Node* lastCstElement;
    Node* newCstElement;
    Node* previousTokenNode;

    bool parseError;
    string parseErrorMessage;
    int parseErrorLine;

    bool nextLinkIsChild;
    int state;
    int numLeftParenthesis;
    int numRightParenthesis;
    int numLeftBraces;
    int numRightBraces;
    int numLeftBrackets;
    int numRightBrackets;

    const Token& currentToken() const;
    const Token& peek(int offset = 0) const;
    bool atEnd() const;
    void advance();
    bool checkType(const string& type, int offset = 0) const;
    bool checkValue(const string& value, int offset = 0) const;
    bool isTypeWord(const Token& t) const;
    bool isReservedWord(const string& word) const;
    Node* createNode(const string& label, const string& text, int line);
    Node* createTokenNode(const Token& t);

    Node* addElementToConcreteSyntaxTree();                          
    void printTreeLines(const Node* start, ostream& out) const;
    void setNextLinkToSibling();
    void setNextLinkToChild();
    void fail(int line, const string& message);
    void destroyTree(Node* n);

public:
    Parser(const vector<Token>& toks);
    ~Parser();
    void printTree(ostream& out) const;
    Node* parse();

    bool hasError() const;
    string getErrorMessage() const;
    int getErrorLine() const;

    Node* getTree() const;
};

#endif