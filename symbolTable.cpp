#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include "parser.cpp"

using namespace std;

class TableEntry
{
public:
    TableEntry(string in, string it, string dt,
               bool ia, int as, int s) : identifierName(in), identifierType(it), datatype(dt), isArray(ia), arraySize(as),
                                         scope(s), next(NULL) {}
    string identifierName;
    string identifierType;
    string datatype;
    bool isArray;
    int arraySize;
    int scope;
    TableEntry *next;
};

class ParameterList
{
public:
    ParameterList(string p, int s) : parentName(p), scope(s), head(NULL), tail(NULL), next(NULL) {}
    string parentName;
    ParameterList *next;

    void addParameter(string identifierName, string datatype, bool isArray, int arraySize)
    {
        TableEntry *temp = new TableEntry(identifierName, "N/A", datatype, isArray, arraySize, scope);
        if (head == NULL)
        {
            head = temp;
            tail = temp;
        }
        else
        {
            tail->next = temp;
            tail = temp;
        }
    }

    TableEntry *getHead()
    {
        return head;
    }

private:
    TableEntry *head;
    TableEntry *tail;
    int scope;
};

class SymbolTable
{
public:
    SymbolTable(CST *c)
    {
        next = c->getHead();
        currentScope = 0;
        highestScope = 0;
        state = START;
        head = NULL;
        tail = NULL;
        paramHead = NULL;
        paramTail = NULL;
        temp = NULL;
    }
    bool formSymbolTable()
    {
        string currentToken;

        while (next != NULL)
        {
            currentToken = next->token();
            // cout << currentToken << " ";
            switch (state)
            {
            case START:
            {
                if (currentToken == "{")
                {
                    // track number of braces
                    braceCount++;
                }
                else if (currentToken == "}")
                {
                    braceCount--;
                    if (braceCount == 0)
                    {
                        // get previous scope
                        currentScope = previousScope.front();
                        previousScope.pop();
                    }
                }
                else if (currentToken == "procedure")
                {
                    // track old scope so we can return when needed
                    previousScope.push(currentScope);
                    // get new unique scope
                    highestScope++;
                    currentScope = highestScope;

                    state = PROCEDURE_NAME;
                    identifierType = "procedure";
                    datatype = "N/A";
                    isArray = false;
                    arraySize = 0;
                }
                else if (currentToken == "function")
                {
                    // track old scope so we can return when needed
                    previousScope.push(currentScope);
                    // get new unique scope
                    highestScope++;
                    currentScope = highestScope;

                    state = FUNCTION_DATATYPE;
                    identifierType = "function";
                    isArray = false;
                    arraySize = 0;
                }
                else if (isDatatype(currentToken))
                {
                    state = VARIABLE_NAME;
                    identifierType = "datatype";
                    datatype = currentToken;
                }
                break;
            }
            case PROCEDURE_NAME:
            {
                identifierName = currentToken;
                state = PROCEDURE_ARGS;
                break;
            }
            case PROCEDURE_ARGS:
            {
                // current token should be '('
                if (!addSymbol())
                {
                    return false;
                }
                state = PARAMETER_LIST_START;
                break;
            }
            case PARAMETER_LIST_START:
            {
                // should either be empty or start with a datatype
                if (currentToken == ")")
                {
                    state = START;
                }
                else if (isDatatype(currentToken))
                {
                    parameterDatatype = currentToken;
                    state = PARAMETER_NAME;
                }
                break;
            }
            case PARAMETER_LIST:
            {
                // could be a datatype or could be another name
                if (isDatatype(currentToken))
                {
                    parameterDatatype = currentToken;
                    state = PARAMETER_NAME;
                }
                else
                {
                    parameterName = currentToken;
                    state = PARAMETER_NEXT;
                }
                break;
            }
            case PARAMETER_NAME:
            {
                parameterName = currentToken;
                state = PARAMETER_NEXT;
                break;
            }
            case PARAMETER_NEXT:
            {
                // check if parameter is an array, if list is done, or if there's another item
                if (currentToken == "[")
                {
                    state = PARAMETER_ARRAY_SIZE;
                    parameterIsArray = true;
                }
                else if (currentToken == ")")
                {
                    // end of list
                    addParameter();
                    addParameterList();
                    state = START;
                }
                else if (currentToken == ",")
                {
                    addParameter();
                    state = PARAMETER_LIST;
                }
                break;
            }
            case PARAMETER_ARRAY_SIZE:
            {
                // should be an integer size
                parameterIsArray = true;
                parameterArraySize = stoi(currentToken);
                state = PARAMETER_ARRAY_END;
                break;
            }
            case PARAMETER_ARRAY_END:
            {
                // symbol should be "]"
                addParameter();
                state = PARAMETER_ARRAY_NEXT;
                break;
            }
            case PARAMETER_ARRAY_NEXT:
            {
                // either "," if list continuing or ")"
                if (currentToken == ")")
                {
                    // end of list
                    addParameterList();
                    state = START;
                }
                else if (currentToken == ",")
                {
                    state = PARAMETER_LIST;
                }
                break;
            }
            case FUNCTION_DATATYPE:
            {
                // should be datatype
                datatype = currentToken;
                state = FUNCTION_NAME;
                break;
            }
            case FUNCTION_NAME:
            {
                // should be function name
                identifierName = currentToken;
                state = FUNCTION_ARGS;
                break;
            }
            case FUNCTION_ARGS:
            {
                // should be "("
                if (!addSymbol())
                {
                    return false;
                }
                state = PARAMETER_LIST_START;
                break;
            }
            case VARIABLE_NAME:
            {
                identifierName = currentToken;
                isArray = false;
                arraySize = 0;
                state = VARIABLE_NEXT;
                break;
            }
            case VARIABLE_NEXT:
            {
                // check if theres a variable list, or if the declaration ended, or if its an array
                if (currentToken == "[")
                {
                    state = VARIABLE_ARRAY_SIZE;
                }
                else if (currentToken == ",")
                {
                    if (!addSymbol())
                    {
                        return false;
                    }
                    state = VARIABLE_NAME;
                }
                else
                {
                    if (!addSymbol())
                    {
                        return false;
                    }
                    state = START;
                    // should be ";"
                }
                break;
            }
            case VARIABLE_ARRAY_SIZE:
            {
                // should be an integer size
                isArray = true;
                arraySize = stoi(currentToken);
                state = VARIABLE_ARRAY_END;
                break;
            }
            case VARIABLE_ARRAY_END:
            {
                // should be "]"
                if (!addSymbol())
                {
                    return false;
                }
                state = VARIABLE_ARRAY_NEXT;
                break;
            }
            case VARIABLE_ARRAY_NEXT:
            {
                if (currentToken == ",")
                {
                    state = VARIABLE_NAME;
                }
                else
                {
                    // should be ";"
                    state = START;
                }
                break;
            }
            default:
                break;
            }

            // go to next entry in cst
            if (next->rightSibling() == NULL)
            {
                next = next->leftChild();
                // cout << endl;
            }
            else
            {
                next = next->rightSibling();
            }
        }
        return true;
    }

    TableEntry *findEntry(string idName)
    {
        TableEntry *curr = head;
        while (curr != NULL)
        {
            if (curr->identifierName == idName)
            {
                return curr;
            }
            curr = curr->next;
        }
        return NULL;
    }

    TableEntry *findParameter(string idName, int scope)
    {
        ParameterList *curr = paramHead;
        TableEntry *currEntry;
        while (curr != NULL)
        {
            currEntry = paramHead->getHead();
            while (currEntry != NULL)
            {
                if (currEntry->identifierName == idName && currEntry->scope == scope)
                {
                    return currEntry;
                }
                currEntry = currEntry->next;
            }
            curr = curr->next;
        }
        return NULL;
    }

    void printTable(ofstream &output)
    {
        TableEntry *current = head;
        while (current != NULL)
        {
            output << "    IDENTIFIER_NAME: " << current->identifierName << endl;
            output << "    IDENTIFIER_TYPE: " << current->identifierType << endl;
            output << "           DATATYPE: " << current->datatype << endl;
            output << "  DATATYPE_IS_ARRAY: ";
            if (current->isArray)
            {
                output << "yes" << endl;
            }
            else
            {
                output << "no" << endl;
            }
            output << "DATATYPE_ARRAY_SIZE: " << current->arraySize << endl;
            output << "              SCOPE: " << current->scope << endl;
            output << endl;
            current = current->next;
        }
        output << endl;
        ParameterList *paramCurrent = paramHead;
        while (paramCurrent != NULL)
        {
            output << " PARAMETER LIST FOR: " << paramCurrent->parentName << endl;
            current = paramCurrent->getHead();
            while (current != NULL)
            {
                output << "    IDENTIFIER_NAME: " << current->identifierName << endl;
                output << "           DATATYPE: " << current->datatype << endl;
                output << "  DATATYPE_IS_ARRAY: ";
                if (current->isArray)
                {
                    output << "yes" << endl;
                }
                else
                {
                    output << "no" << endl;
                }
                output << "DATATYPE_ARRAY_SIZE: " << current->arraySize << endl;
                output << "              SCOPE: " << current->scope << endl;
                output << endl;
                current = current->next;
            }
            paramCurrent = paramCurrent->next;
        }
        return;
    }

private:
    TreeNode *next;
    int currentScope;
    int highestScope;
    int braceCount = 0;
    queue<int> previousScope;
    TableEntry *head;
    TableEntry *tail;

    string identifierName;
    string identifierType;
    string datatype;
    bool isArray;
    int arraySize;

    string parameterName;
    string parameterDatatype;
    bool parameterIsArray;
    int parameterArraySize;

    ParameterList *temp;
    ParameterList *paramHead;
    ParameterList *paramTail;

    enum StateLabel
    {
        START,
        PROCEDURE_NAME,
        PROCEDURE_ARGS,
        PARAMETER_LIST_START,
        PARAMETER_LIST,
        PARAMETER_NAME,
        PARAMETER_NEXT,
        PARAMETER_ARRAY_SIZE,
        PARAMETER_ARRAY_END,
        PARAMETER_ARRAY_NEXT,
        FUNCTION_DATATYPE,
        FUNCTION_NAME,
        FUNCTION_ARGS,
        VARIABLE_NAME,
        VARIABLE_NEXT,
        VARIABLE_ARRAY_SIZE,
        VARIABLE_ARRAY_END,
        VARIABLE_ARRAY_NEXT
    };

    StateLabel state;

    bool isDatatype(string token)
    {
        vector<string> datatypes = {"char", "bool", "int"};
        bool isValid = false;
        for (int i = 0; i < datatypes.size(); i++)
        {
            if (token == datatypes.at(i))
            {
                isValid = true;
                break;
            }
        }
        return isValid;
    }

    bool addSymbol()
    {
        TableEntry *temp = new TableEntry(identifierName, identifierType, datatype, isArray, arraySize, currentScope);
        TableEntry *check;
        check = findEntry(identifierName);
        if (check != NULL)
        {
            if (check->scope == 0)
            {
                cout << "Error on line " << next->getLine() << ": variable \"" << identifierName << "\" is already defined globally." << endl;
                return false;
            }
            else if (check->scope == currentScope)
            {
                cout << "Error on line " << next->getLine() << ": variable \"" << identifierName << "\" is already defined locally." << endl;
                return false;
            }
        }
        check = findParameter(identifierName, currentScope);
        if (check != NULL)
        {
            cout << "Error on line " << next->getLine() << ": variable \"" << identifierName << "\" is already defined locally." << endl;
            return false;
        }
        if (head == NULL)
        {
            head = temp;
            tail = temp;
        }
        else
        {
            tail->next = temp;
            tail = temp;
        }
        return true;
    }

    void addParameter()
    {
        if (temp == NULL)
        {
            temp = new ParameterList(identifierName, currentScope);
        }
        temp->addParameter(parameterName, parameterDatatype, parameterIsArray, parameterArraySize);
    }

    void addParameterList()
    {
        if (paramHead == NULL)
        {
            paramHead = temp;
            paramTail = temp;
            temp = NULL;
        }
        else
        {
            paramTail->next = temp;
            paramTail = temp;
            temp = NULL;
        }
    }
};