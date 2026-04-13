#ifndef TOKENIZATION_H
#define TOKENIZATION_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;

struct Token{
    string type;
    string value;
    int line;
};

struct TokenizeResult {
    bool error = false;
    int errorLine=0;
    string errorMessage = "";
    std::vector<Token> tokens;
};

TokenizeResult tokenization(std::string str);

#endif // TOKENIZATION_H