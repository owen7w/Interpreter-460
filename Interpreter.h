#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <iostream>
#include <string>
#include <vector>
#include "Node.h"
#include "SymbolTable.h"

using namespace std;

// Holds a value while the program is running.
struct RuntimeValue {
    string datatype; // "int", "char", "bool"

    int intValue;
    char charValue;
    bool boolValue;
    string stringValue;

    RuntimeValue()
        : datatype(""),
          intValue(0),
          charValue('\0'),
          boolValue(false),
          stringValue("")
    {
    }
};

class Interpreter {
private:
    Node* astRoot;
    SymbolNode* symbolTableHead;
    ostream& out;

    int currentScope;
    bool isReturning;
    RuntimeValue lastReturnValue;

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

    // Evaluates a postfix expression using a stack.
    RuntimeValue evaluateExpression(Node* exprNode);

    // Finds a function/procedure in the symbol table.
    SymbolNode* findFunctionSymbol(const string& name);

    // Executes a function/procedure call.
    RuntimeValue executeFunction(const string& funcName, const vector<RuntimeValue>& args);

    // Finds a variable/function using the current scope.
    SymbolNode* lookupSymbol(const string& name);

    // Reads a symbol table value into a RuntimeValue.
    RuntimeValue getSymbolValue(SymbolNode* symbol);

    // Reads a variable by name.
    RuntimeValue getVariableValue(const string& name);

    // Stores a RuntimeValue into a symbol table entry.
    void assignValue(SymbolNode* symbol, int index, const RuntimeValue& value);

public:
    Interpreter(Node* astRoot, SymbolNode* symbolTableHead, ostream& out);

    // Starts running the program.
    void run();
};

#endif