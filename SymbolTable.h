#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <string>
#include <iostream>
#include "Node.h"

using namespace std;

struct ParameterNode { // keeping paramenter separate from symbols for easier handling of parameter lists as we are printing also and where it tied to
    string name;
    string datatype;
    bool isArray;
    int arraySize;
    int scope;
    ParameterNode* next;

    ParameterNode(const string& n = "", const string& dt = "", bool arr = false,int arrSize = 0, int sc = 0) 
    : name(n), datatype(dt), isArray(arr), arraySize(arrSize), scope(sc), next(nullptr) {}
};

struct SymbolNode { // main thing that we will strung together to make symbol table hold the all the info
    string name;         
    string identifierType;  
    string datatype;        
    bool isArray;
    int arraySize;
    int scope;
    int lineDeclared;

    ParameterNode* parameterList;
    SymbolNode* next;

    SymbolNode(const string& n = "", const string& idType = "", const string& dt = "", bool arr = false, int arrSize = 0, int sc = 0, int line = 0)
        : name(n), identifierType(idType), datatype(dt), isArray(arr), arraySize(arrSize), scope(sc), lineDeclared(line), parameterList(nullptr), next(nullptr) {}
};

struct SymbolTableResult { // struct to hold the result of symbol table creation, including any error information
    bool error = false;
    string errorMessage = "";
    SymbolNode* head = nullptr;
};

SymbolTableResult createSymbolTable(Node* concreteSyntaxTree);
void printSymbolTable(SymbolNode* head, ostream& out);
void freeSymbolTable(SymbolNode* head);

#endif