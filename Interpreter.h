#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <iostream>
#include <string>
#include <vector>
#include "Node.h"
#include "SymbolTable.h"

using namespace std;

class Interpreter {
private:
    Node* astRoot;
    SymbolNode* symbolTableHead;
    ostream& out;

    int currentScope;
    bool isReturning;
    int lastReturnValue;

    // Finds the main procedure in the AST.
    Node* findMainProcedure();

    // Skips from BEGIN BLOCK to the matching END BLOCK.
    Node* skipBlock(Node* beginBlockNode);

    // Finds the next statement line in the AST.
    Node* getNextLine(Node* current);

    // Executes a BEGIN/END block.
    void executeBlock(Node* blockNode);

    // Executes an assignment statement.
    void executeAssignment(Node* assignmentNode);

    // Executes a printf statement.
    void executePrintf(Node* printfNode);

    // Executes an if statement.
    void executeIf(Node* ifNode);

    // Executes a while loop.
    void executeWhile(Node* whileNode);

    // Executes a for loop.
    void executeFor(Node* forNode);

    // Executes a return statement.
    void executeReturn(Node* returnNode);

    // Evaluates a postfix integer expression using a stack.
    int evaluateExpression(Node* exprNode);

    // Finds a function/procedure in the symbol table.
    SymbolNode* findFunctionSymbol(const string& name);

    // Executes a function/procedure call.
    int executeFunction(const string& funcName, const vector<int>& args);

    // Finds a variable/function using the current scope.
    SymbolNode* lookupSymbol(const string& name);

    // Reads an int value from a symbol.
    int getSymbolIntValue(SymbolNode* symbol);

    // Reads an int variable by name.
    int getVariableIntValue(const string& name);

    // Stores an int value into a symbol table entry.
    void assignIntValue(SymbolNode* symbol, int index, int value);


        // Reads a char value from a symbol.
    char getSymbolCharValue(SymbolNode* symbol);

    // Reads a char variable by name.
    char getVariableCharValue(const string& name);

    // Stores a char value into a symbol table entry.
    void assignCharValue(SymbolNode* symbol, int index, char value);

    // Evaluates a char expression or char literal.
    char evaluateCharExpression(Node* exprNode);


    // Evaluates function/procedure call arguments.
    vector<int> evaluateArgumentList(Node* openParenNode);


public:
    Interpreter(Node* astRoot, SymbolNode* symbolTableHead, ostream& out);

    // Starts running the program.
    void run();
};

#endif