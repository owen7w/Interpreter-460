#ifndef AST_H
#define AST_H

#include <iostream>
#include <string>
#include <vector>
#include "Node.h"

using namespace std;

class AST {
private:
    Node* concreteSyntaxTree;
    Node* abstractSyntaxTree;

    Node* lastAstElement;
    bool buildError;
    string buildErrorMessage;
    int buildErrorLine;
    bool nextLinkIsChild;


    void printTreeLines(const Node* start, ostream& out) const;
    void destroyTree(Node* n);

    Node* createAstNode(const string& label, const string& text = "", int line = 0);
    Node* addElementToAbstractSyntaxTree(const string& label,
                                         const string& text = "",
                                         int line = 0);

    void setNextLinkToSibling();
    void setNextLinkToChild();
    void shuntingYard(const vector<Node*>& list); 
public:
    AST();
    ~AST();

    Node* build(Node* cstRoot);
    Node* getTree() const;

    bool hasError() const;
    string getErrorMessage() const;
    int getErrorLine() const;

    void printTree(ostream& out) const;
};

#endif