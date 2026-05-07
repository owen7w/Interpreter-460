#include "Interpreter.h"
#include <stdexcept>

using namespace std;

Interpreter::Interpreter(Node* astRoot, SymbolNode* symbolTableHead, ostream& out)
    : astRoot(astRoot),
      symbolTableHead(symbolTableHead),
      out(out),
      currentScope(0),
      isReturning(false),
      lastReturnValue()
{
}

void Interpreter::run()
{
    Node* mainNode = findMainProcedure();

    if (mainNode == nullptr)
    {
        throw runtime_error("main procedure not found");
    }

    Node* mainBlock = getNextLine(mainNode);

    if (mainBlock == nullptr || mainBlock->label != "BEGIN BLOCK")
    {
        throw runtime_error("main procedure has no BEGIN BLOCK");
    }

    executeBlock(mainBlock);
}

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

void Interpreter::executeBlock(Node* blockNode)
{
    if (blockNode == nullptr || blockNode->label != "BEGIN BLOCK")
    {
        return;
    }

    Node* current = getNextLine(blockNode);

    while (current != nullptr && !isReturning)
    {
        StatementKind kind = getStatementKind(current);

        switch (kind)
        {
            case StatementKind::ASSIGNMENT:
                out << "Found assignment" << endl;
                // executeAssignment(current);
                current = getNextLine(current);
                break;

            case StatementKind::PRINTF:
                out << "Found printf" << endl;
                // executePrintf(current);
                current = getNextLine(current);
                break;

            case StatementKind::IF:
                out << "Found if" << endl;
                // executeIf(current);
                current = getNextLine(current);
                break;

            case StatementKind::WHILE:
                out << "Found while" << endl;
                // executeWhile(current);
                current = getNextLine(current);
                break;

            case StatementKind::FOR:
                out << "Found for" << endl;
                // executeFor(current);
                current = getNextLine(current);
                break;

            case StatementKind::RETURN:
                out << "Found return" << endl;
                // executeReturn(current);
                current = getNextLine(current);
                break;

            case StatementKind::CALL:
                out << "Found function/procedure call" << endl;
                // executeFunction(???);
                current = getNextLine(current);
                break;

            case StatementKind::BEGIN_BLOCK:
                out << "Found nested block" << endl;
                executeBlock(current);
                current = getNextLine(current);
                break;

            case StatementKind::END_BLOCK:
                out << "End of block" << endl;
                return;

            case StatementKind::DECLARATION:
                out << "Found declaration" << endl;
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

SymbolNode* Interpreter::lookupSymbol(const string& name)
{
    SymbolNode* symbol = findSymbol(symbolTableHead, name, currentScope);

    if (symbol == nullptr)
    {
        throw runtime_error("Undefined identifier: " + name);
    }

    return symbol;
}


RuntimeValue Interpreter::getSymbolValue(SymbolNode* symbol)
{
    if (symbol == nullptr)
    {
        throw runtime_error("Cannot read null symbol");
    }

    RuntimeValue value;
    value.datatype = symbol->datatype;

    if (symbol->datatype == "int")
    {
        value.intValue = getIntValue(symbol);
    }
    else if (symbol->datatype == "char")
    {
        value.charValue = getCharValue(symbol);
    }
    else if (symbol->datatype == "bool")
    {
        value.boolValue = getBoolValue(symbol);
    }
    else
    {
        throw runtime_error("I have not made this " + symbol->datatype);
    }

    return value;
}


RuntimeValue Interpreter::getVariableValue(const string& name)
{
    SymbolNode* symbol = lookupSymbol(name);
    return getSymbolValue(symbol);
}


void Interpreter::assignValue(SymbolNode* symbol, int index, const RuntimeValue& value)
{
    if (symbol == nullptr)
    {
        throw runtime_error("Cannot assign to null symbol");
    }

    if (index >= 0)
    {
        if (symbol->datatype == "int")
        {
            setIntArrayValue(symbol, index, value.intValue);
        }
        else if (symbol->datatype == "char")
        {
            setCharArrayValue(symbol, index, value.charValue);
        }
        else if (symbol->datatype == "bool")
        {
            setBoolArrayValue(symbol, index, value.boolValue);
        }
        else
        {
            throw runtime_error("Unsupported array assignment type: " + symbol->datatype);
        }

        return;
    }

    if (symbol->datatype == "int")
    {
        setIntValue(symbol, value.intValue);
    }
    else if (symbol->datatype == "char")
    {
        setCharValue(symbol, value.charValue);
    }
    else if (symbol->datatype == "bool")
    {
        setBoolValue(symbol, value.boolValue);
    }
    else
    {
        throw runtime_error("Unsupported assignment type: " + symbol->datatype);
    }
}






