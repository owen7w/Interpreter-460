#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

#include "ignoreComments.h"
#include "Tokenization.h"
#include "Parser.h"
#include "SymbolTable.h"
#include "AST.h"
#include "Interpreter.h"

int main(int argc, const char** argv) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <input_file>" << endl;
        return 1;
    }

    ifstream input_file(argv[1], ios::binary);
    if (!input_file.is_open()) {
        cout << "Error opening file: " << argv[1] << endl;
        return 1;
    }

    fs::path inPath(argv[1]);
    fs::path outPath = inPath;
    outPath.replace_filename("output-" + outPath.stem().string() + ".txt");

    ofstream out(outPath, ios::binary);
    if (!out.is_open()) {
        cout << "Error creating output file: " << outPath.string() << endl;
        return 1;
    }

    string str((istreambuf_iterator<char>(input_file)),
               istreambuf_iterator<char>());
    input_file.close();

    auto cr = ignoreComments(str);
    if (cr.unterminated) {
        out << "ERROR: Program contains C-style, unterminated comment on line "
            << cr.unterminatedLine << "\n";
        return 1;
    }

    TokenizeResult tr = tokenization(cr.text);
    if (tr.error) {
        if (!tr.errorMessage.empty()) {
            out << tr.errorMessage << "\n";
        } else {
            out << "ERROR: Program contains invalid token on line " << tr.errorLine << "\n";
        }
        return 1;
    }

    Parser parser(tr.tokens);
    Node* tree = parser.parse();

    if (parser.hasError()) {
        out << parser.getErrorMessage() << "\n";
        return 1;
    }

    AST ast;
    Node* astRoot = ast.build(tree);

    if (ast.hasError() || astRoot == nullptr) {
        if (!ast.getErrorMessage().empty()) {
            out << ast.getErrorMessage() << "\n";
        } else {
            out << "ERROR: Failed to build AST\n";
        }
        return 1;
    }

    SymbolTableResult stResult = createSymbolTable(tree);
    if (stResult.error) {
        out << stResult.errorMessage << "\n";
        return 1;
    }

    Interpreter interpreter(astRoot, stResult.head, out);
    interpreter.run();

    freeSymbolTable(stResult.head);

    return 0;
}