#include "SymbolTable.h"

static Node* advanceCST(Node* cur) {
    if (cur == nullptr) {
        return nullptr;
    }
    if (cur->sibling != nullptr) {
        return cur->sibling;
    }
    return cur->child;
}

static bool isDatatypeWord(const string& s) {
    return s == "int" ||
           s == "char" ||
           s == "bool" ||
           s == "void" ||
           s == "float" ||
           s == "double";
}

static void setError(SymbolTableResult& result, int line, const string& msg) {
    result.error = true;
    result.errorMessage = "Error on line " + to_string(line) + ": " + msg;
}

static ParameterNode* appendParameter(ParameterNode*& head, const ParameterNode& value) {
    ParameterNode* node = new ParameterNode(value);

    if (head == nullptr) {
        head = node;
        return node;
    }

    ParameterNode* cur = head;
    while (cur->next != nullptr) {
        cur = cur->next;
    }
    cur->next = node;
    return node;
}

static void freeParameterList(ParameterNode* head) {
    while (head != nullptr) {
        ParameterNode* next = head->next;
        delete head;
        head = next;
    }
}

static SymbolNode* appendSymbol(SymbolNode*& head, const SymbolNode& value) {
    SymbolNode* node = new SymbolNode(value);

    if (head == nullptr) {
        head = node;
        return node;
    }

    SymbolNode* cur = head;
    while (cur->next != nullptr) {
        cur = cur->next;
    }
    cur->next = node;
    return node;
}

static bool existsInScope(SymbolNode* head, const string& name, int scope) {
    for (SymbolNode* cur = head; cur != nullptr; cur = cur->next) {
        if (cur->name == name && cur->scope == scope) {
            return true;
        }
    }
    return false;
}

static bool existsGlobally(SymbolNode* head, const string& name) {
    return existsInScope(head, name, 0);
}











SymbolTableResult createSymbolTable(Node* concreteSyntaxTree) {
    SymbolTableResult result;

    Node* cst = concreteSyntaxTree;

    int state = 0;
    int currentScope = 0;
    int nextRoutineScope = 1;
    int braceDepth = 0;

    string pendingType;
    string pendingName;
    int pendingLine = 0;
    bool pendingIsArray = false;
    int pendingArraySize = 0;

    string currentRoutineName;
    string currentRoutineType;
    string currentRoutineDatatype;
    SymbolNode* currentRoutineNode = nullptr;

    while (cst != nullptr && !result.error) {
        const string token = cst->text;
        const int line = cst->line;

        switch (state) {
            case 0:
                if (token == "function") {
                state = 1000;
                cst = advanceCST(cst);
                }
                else if (token == "procedure") {
                    state = 2000;
                    cst = advanceCST(cst);
                }
                else {
                    pendingType = token;
                    state = 100;
                    cst = advanceCST(cst);
                }
                break;

            case 100:
                pendingName = token;
                pendingLine = line;
                pendingIsArray = false;
                pendingArraySize = 0;
                state = 101;
                cst = advanceCST(cst);
                break;

            case 101:
                if (token == "[") { // array declaration
                    pendingIsArray = true;
                    state = 102;
                    cst = advanceCST(cst);
                }
                else if (token == ",") { // another variable in this declaration
                    if (existsInScope(result.head, pendingName, 0)) {
                        setError(result, pendingLine, "variable \"" + pendingName + "\" is already defined globally");
                    }
                    else {
                        appendSymbol(result.head, SymbolNode(pendingName, "datatype", pendingType, pendingIsArray, pendingArraySize, 0, pendingLine));
                    }
                    state = 100;
                    cst = advanceCST(cst);
                }
                else { // end of declaration
                    if (existsInScope(result.head, pendingName, 0)) {
                        setError(result, pendingLine, "variable \"" + pendingName + "\" is already defined globally");
                    }
                    else {
                        appendSymbol(result.head, SymbolNode(pendingName, "datatype", pendingType, pendingIsArray, pendingArraySize, 0, pendingLine));
                    }
                    state = 0;
                    cst = advanceCST(cst);
                }
                break;

            case 102: // array size
                pendingArraySize = stoi(token);
                state = 103;
                cst = advanceCST(cst);
                break;


            case 103: // ]
                state = 101;
                cst = advanceCST(cst);
                break;



            case 1000:
                currentRoutineType = "function";
                currentRoutineDatatype = token;   // return type
                state = 1001;
                cst = advanceCST(cst);
                break;

            case 1001:
                currentRoutineName = token;
                pendingLine = line;
                currentScope = nextRoutineScope++;
                if (existsGlobally(result.head, currentRoutineName)) {
                    setError(result, pendingLine, currentRoutineType + " \"" + currentRoutineName + "\" is already defined globally");
                }
                else {
                    currentRoutineNode = appendSymbol(result.head, SymbolNode(currentRoutineName, currentRoutineType, currentRoutineDatatype, false, 0, currentScope, pendingLine));
                }
                state = 1002;
                cst = advanceCST(cst);
                break;

            case 1002: // (
                state = 1003;
                cst = advanceCST(cst);
                break;

            case 1003: // parameter list or )
                if (token == ")") {
                    state = 1008;
                    cst = advanceCST(cst);
                }
                else if(token == "void") {
                    cst = advanceCST(cst); // void
                    state = 1008;
                    cst = advanceCST(cst); // )
                }
                else{ // parameter datatype
                    pendingType = token;
                    state = 1004;
                    cst = advanceCST(cst);
                }

                break;

            case 1004: // parameter name
                pendingName = token;
                pendingLine = line;
                pendingIsArray = false;
                pendingArraySize = 0;
                state = 1005;
                cst = advanceCST(cst);
                break;
            
            case 1005: // parameter name followed by , or ) or [ for array
                if (token == "[") {
                    pendingIsArray = true;
                    state = 1006;
                    cst = advanceCST(cst);
                }
                else {
                    if (existsGlobally(result.head, pendingName)) {
                        setError(result, pendingLine, "variable \"" + pendingName + "\" is already defined globally");
                    }
                    else if (existsInScope(result.head, pendingName, currentScope)) {
                        setError(result, pendingLine, "variable \"" + pendingName + "\" is already defined locally");
                    }
                    else {
                        appendParameter(currentRoutineNode->parameterList, ParameterNode(pendingName, pendingType, pendingIsArray, pendingArraySize, currentScope));
                        appendSymbol(result.head, SymbolNode(pendingName, "parameter", pendingType, pendingIsArray, pendingArraySize, currentScope, pendingLine));
                    }
                    if (token == ",") {
                        state = 1003;
                        cst = advanceCST(cst);
                    }        
                    else if (token == ")") {
                        state = 1008;
                        cst = advanceCST(cst);
                    }
                }
                break;


            case 1006: // parameter array size
                pendingArraySize = stoi(token);
                state = 1007;
                cst = advanceCST(cst); //#
                break;
            
            case 1007: // ]
                state = 1005;
                cst = advanceCST(cst); // }
                break;


            case 1008: // into the body
                braceDepth = 1;
                state = 3000;
                cst = advanceCST(cst); // {
                break;

            
                





            case 2000: // no need to redo everything so I used the same states for function parameters
                currentRoutineType = "procedure";
                currentRoutineDatatype = "NOT APPLICABLE";
                currentRoutineName = token;
                pendingLine = line;
                currentScope = nextRoutineScope++;
                if (existsGlobally(result.head, currentRoutineName)) {
                    setError(result, pendingLine, currentRoutineType + " \"" + currentRoutineName + "\" is already defined globally");
                }
                else {
                    currentRoutineNode = appendSymbol(result.head, SymbolNode(currentRoutineName, currentRoutineType, currentRoutineDatatype, false, 0, currentScope, pendingLine));
                }
                state = 1002;
                cst = advanceCST(cst);
                break;







            case 3000: // inside function/procedure body
                if (token == "{") {
                    braceDepth++;
                    cst = advanceCST(cst);
                }
                else if (token == "}") {
                    braceDepth--;
                    if (braceDepth == 0) { // end of function/procedure body
                        currentScope = 0;
                        currentRoutineName = "";
                        currentRoutineType = "";
                        currentRoutineDatatype = "";
                        currentRoutineNode = nullptr;
                        state = 0;
                    }
                    cst = advanceCST(cst);
                }
                else if (isDatatypeWord(token) && token != "void") {
                    pendingType = token;
                    state = 3100;
                    cst = advanceCST(cst);
                }
                else {
                    cst = advanceCST(cst);
                }
                break;

            case 3100: // variable declaration in function/procedure body
                pendingName = token;
                pendingLine = line;
                pendingIsArray = false;
                pendingArraySize = 0;
                state = 3101;
                cst = advanceCST(cst);
                break;
            
            case 3101:
                if (token == "[") { // array declaration
                    pendingIsArray = true;
                    state = 3102;
                    cst = advanceCST(cst);
                }
                else if (token == ",") { // another variable in this declaration
                    if (existsInScope(result.head, pendingName, currentScope)) {
                        setError(result, pendingLine, "variable \"" + pendingName + "\" is already defined locally");
                    }
                    else if (existsGlobally(result.head, pendingName)) {
                        setError(result, pendingLine, "variable \"" + pendingName + "\" is already defined globally");
                    }
                    else {
                        appendSymbol(result.head, SymbolNode(pendingName, "datatype", pendingType, pendingIsArray, pendingArraySize, currentScope, pendingLine));
                    }
                    state = 3100;
                    cst = advanceCST(cst);
                }
                else { 
                    if (existsInScope(result.head, pendingName, currentScope)) {
                        setError(result, pendingLine, "variable \"" + pendingName + "\" is already defined locally");
                    }
                    else if (existsGlobally(result.head, pendingName)) {
                        setError(result, pendingLine, "variable \"" + pendingName + "\" is already defined globally");
                    }
                    else {
                        appendSymbol(result.head, SymbolNode(pendingName, "datatype", pendingType, pendingIsArray, pendingArraySize, currentScope, pendingLine));
                    }
                    state = 3000;
                    cst = advanceCST(cst);
                }
                break;
            
            case 3102: // array size
                pendingArraySize = stoi(token);
                state = 3103;
                cst = advanceCST(cst);
                break;
                
            case 3103: // ]
                state = 3101;
                cst = advanceCST(cst);
                break;







            default:
                setError(result, line, "symbol table state error");
                break;
            
        }
    }

    if (result.error) {
        freeSymbolTable(result.head);
        result.head = nullptr;
    }

    return result;
}

void printSymbolTable(SymbolNode* head, ostream& out) {
    for (SymbolNode* cur = head; cur != nullptr; cur = cur->next) {
        if(cur->identifierType == "parameter") {
            continue;
        }

        out << "      IDENTIFIER_NAME: " << cur->name << "\n";
        out << "      IDENTIFIER_TYPE: " << cur->identifierType << "\n";
        out << "             DATATYPE: " << cur->datatype << "\n";
        out << "    DATATYPE_IS_ARRAY: " << (cur->isArray ? "yes" : "no") << "\n";
        out << "  DATATYPE_ARRAY_SIZE: " << cur->arraySize << "\n";
        out << "                SCOPE: " << cur->scope << "\n\n";
    }

    for (SymbolNode* cur = head; cur != nullptr; cur = cur->next) {
        if (cur->parameterList == nullptr) {
            continue;
        }

        out << "\n   PARAMETER LIST FOR: " << cur->name << "\n";

        for (ParameterNode* p = cur->parameterList; p != nullptr; p = p->next) {
            out << "      IDENTIFIER_NAME: " << p->name << "\n";
            out << "             DATATYPE: " << p->datatype << "\n";
            out << "    DATATYPE_IS_ARRAY: " << (p->isArray ? "yes" : "no") << "\n";
            out << "  DATATYPE_ARRAY_SIZE: " << p->arraySize << "\n";
            out << "                SCOPE: " << p->scope << "\n\n";
        }
    }
}

void freeSymbolTable(SymbolNode* head) {
    while (head != nullptr) {
        SymbolNode* next = head->next;
        freeParameterList(head->parameterList);
        delete head;
        head = next;
    }
}