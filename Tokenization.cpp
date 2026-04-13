#include <iostream>
#include <fstream>
#include <string>
#include <sstream> 
#include <cctype>
using namespace std;   
#include "Tokenization.h"


TokenizeResult tokenization(std::string str) {
    TokenizeResult result;
    size_t i = 0;
    int state = 0;  // 0 = whitespace, 1 = Identifier, 2 = Integer, 3 = single quote string, 4 = double quote string, 6 = error
    int lineNumber = 1;
    int tokenStartLine = 1;
    string currentToken = "";
    string currentType = "";

     auto emit = [](string type, string value, int line, TokenizeResult& result) {
        Token token;
        token.type = type;
        token.value = value;
        token.line = line;
        result.tokens.push_back(token);
    };
    while(i < str.length()){
        char c = str[i];
        switch(state) {
            case 0:
                if(c == '\n') {
                    lineNumber++;
                    i++;
                    break;
                }
                else if(c == ' ' || c == '\t' || c == '\r') {
                    // stay in whitespace
                    i++;
                    break;
                }
                else if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_') {
                    currentToken.clear();
                    currentToken.push_back(c);
                    tokenStartLine = lineNumber;
                    state = 1; // Identifier
                    i++;
                    break;
                }
                else if((c=='+' || c=='-') && (i+1 < str.size()) && (str[i+1] >= '0' && str[i+1] <= '9') ) {
                    currentToken.clear();
                    currentToken.push_back(c);
                    tokenStartLine = lineNumber;
                    state = 2; // +-Integer
                    i++;
                    break;
                }
                else if(c >= '0' && c <= '9') {
                    currentToken.clear();
                    currentToken.push_back(c);
                    tokenStartLine = lineNumber;
                    state = 2; // Integer
                    i++;
                    break;
                }
                else if(c == '\'') {
                    currentToken.clear();
                    currentToken.push_back(c);
                    tokenStartLine = lineNumber;
                    emit("SINGLE_QUOTE", "\'", lineNumber, result);
                    currentToken.clear();
                    state = 3; // single quote string
                    i++;
                    break;
                }
                else if(c == '"') {
                    currentToken.clear();
                    tokenStartLine = lineNumber;
                    emit("DOUBLE_QUOTE", "\"", lineNumber, result);
                    state = 4; // double quote string
                     i++;
                    break;
                }
                else if (c=='<' && i+1<str.size() && str[i+1]=='=') { emit("LT_EQUAL","<=",lineNumber,result); i+=2; break; }
                else if (c=='>' && i+1<str.size() && str[i+1]=='=') { emit("GT_EQUAL",">=",lineNumber,result); i+=2; break; }
                else if (c=='=' && i+1<str.size() && str[i+1]=='=') { emit("BOOLEAN_EQUAL","==",lineNumber,result); i+=2; break; }
                else if (c=='!' && i+1<str.size() && str[i+1]=='=') { emit("BOOLEAN_NOT_EQUAL","!=",lineNumber,result); i+=2; break; }
                else if (c=='&' && i+1<str.size() && str[i+1]=='&') { emit("BOOLEAN_AND","&&",lineNumber,result); i+=2; break; }
                else if (c=='|' && i+1<str.size() && str[i+1]=='|') { emit("BOOLEAN_OR","||",lineNumber,result); i+=2; break; }     
                else switch(c){
                    case '(': emit("L_PAREN","(",lineNumber,result); i++; break;
                    case ')': emit("R_PAREN",")",lineNumber,result); i++; break;
                    case '{': emit("L_BRACE","{",lineNumber,result); i++; break;
                    case '}': emit("R_BRACE","}",lineNumber,result); i++; break;
                    case '[': emit("L_BRACKET","[",lineNumber,result); i++; break;
                    case ']': emit("R_BRACKET","]",lineNumber,result); i++; break;
                    case ';': emit("SEMICOLON",";",lineNumber,result); i++; break;
                    case ',': emit("COMMA",",",lineNumber,result); i++; break;
                    case '+': emit("PLUS","+",lineNumber,result); i++; break;
                    case '-': emit("MINUS","-",lineNumber,result); i++; break;
                    case '*': emit("ASTERISK","*",lineNumber,result); i++; break;
                    case '/': emit("DIVIDE","/",lineNumber,result); i++; break;
                    case '%': emit("MODULO","%",lineNumber,result); i++; break;
                    case '<': emit("LT","<",lineNumber,result); i++; break;
                    case '>': emit("GT",">",lineNumber,result); i++; break;
                    case '=': emit("ASSIGNMENT_OPERATOR","=",lineNumber,result); i++; break;
                    case '!': emit("BOOLEAN_NOT","!",lineNumber,result); i++; break;
                    default:
                        result.error = true;
                        result.errorLine = lineNumber;
                        return result;
                }
                break;
            case 1:
                if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || (c >= '0' && c <= '9')) {
                    currentToken.push_back(c);
                    i++;
                    break;
                } else {
                    emit("IDENTIFIER", currentToken, tokenStartLine, result);
                    currentToken.clear();
                    state = 0;
                }
                break;
            case 2:
                if(c >= '0' && c <= '9') {
                    currentToken.push_back(c);
                    i++;
                    break;
                } else {
                    if (i < str.size() && (isalpha(c) || c=='_' || c=='.')) {
                        result.error = true; 
                        result.errorLine = lineNumber;
                        return result;
                        break;
                    }
                    emit("INTEGER", currentToken, tokenStartLine, result);
                    currentToken = "";
                    state = 0;
                }
                break;



            case 3:
                while (i < str.size() && str[i] != '\'') {
                    if (str[i] == '\n') {
                        result.error = true;
                        result.errorLine = tokenStartLine;
                        return result;
                    }

                    if (str[i] == '\\') {

                        if (!currentToken.empty()) {
                            emit("STRING", currentToken, tokenStartLine, result);
                            currentToken.clear();
                        }

                        string escapeSequence;
                        escapeSequence.push_back(str[i]);
                        i++;

                        if (i >= str.size()) {
                            result.error = true;
                            result.errorLine = tokenStartLine;
                            return result;
                        }

                        if (str[i] == 'a' || str[i] == 'b' || str[i] == 'f' ||
                            str[i] == 'n' || str[i] == 'r' || str[i] == 't' ||
                            str[i] == 'v' || str[i] == '\\' || str[i] == '?' ||
                            str[i] == '\'' || str[i] == '\"') {

                            escapeSequence.push_back(str[i]);
                            emit("ESCAPED_CHARACTER", escapeSequence, tokenStartLine, result);
                            i++;
                            continue;
                        }

                        else if (str[i] == 'x') {
                            escapeSequence.push_back(str[i]);
                            i++;

                            if (i >= str.size()) {
                                result.error = true;
                                result.errorLine = tokenStartLine;
                                return result;
                            }

                            auto isHex = [](char ch) {
                                return (ch >= '0' && ch <= '9') ||
                                    (ch >= 'A' && ch <= 'F') ||
                                    (ch >= 'a' && ch <= 'f');
                            };

                            if (!isHex(str[i])) {
                                result.error = true;
                                result.errorLine = tokenStartLine;
                                return result;
                            }

                            escapeSequence.push_back(str[i]);
                            i++;

                            if (i < str.size() && isHex(str[i])) {
                                escapeSequence.push_back(str[i]);
                                i++;
                            }

                            emit("ESCAPED_CHARACTER", escapeSequence, tokenStartLine, result);
                            continue;
                        }
                        else {
                            result.error = true;
                            result.errorLine = tokenStartLine;
                            return result;
                        }
                    }

                    currentToken.push_back(str[i]);
                    i++;
                }

                if (i >= str.size()) {
                    result.error = true;
                    result.errorLine = tokenStartLine;
                    return result;
                }


                if (!currentToken.empty()) {
                    emit("STRING", currentToken, tokenStartLine, result);
                    currentToken.clear();
                }

                emit("SINGLE_QUOTE", "\'", lineNumber, result);
                state = 0;
                i++;
                break;






            case 4:
                while (i < str.size() && str[i] != '"') {
                    if (str[i] == '\n') {
                        result.error = true;
                        result.errorLine = tokenStartLine;
                        result.errorMessage =
                            "Syntax error on line " + to_string(tokenStartLine) +
                            ": unterminated string quote.";
                        return result;
                    }

                    if (str[i] == '\\') {
                        if (!currentToken.empty()) {
                            emit("STRING", currentToken, tokenStartLine, result);
                            currentToken.clear();
                        }

                        string escapeSequence;
                        escapeSequence.push_back(str[i]);
                        i++;

                        if (i >= str.size()) {
                            result.error = true;
                            result.errorLine = tokenStartLine;
                            result.errorMessage =
                                "Syntax error on line " + to_string(tokenStartLine) +
                                ": unterminated string quote.";
                            return result;
                        }


                        if (str[i] == 'a' || str[i] == 'b' || str[i] == 'f' ||
                            str[i] == 'n' || str[i] == 'r' || str[i] == 't' ||
                            str[i] == 'v' || str[i] == '\\' || str[i] == '?' ||
                            str[i] == '\'' || str[i] == '\"') {

                            escapeSequence.push_back(str[i]);
                            emit("ESCAPED_CHARACTER", escapeSequence, tokenStartLine, result);
                            i++;
                            continue;
                        }
                        else if (str[i] == 'x') {
                            escapeSequence.push_back(str[i]);
                            i++;

                            if (i >= str.size()) {
                                result.error = true;
                                result.errorLine = tokenStartLine;
                                result.errorMessage =
                                    "Syntax error on line " + to_string(tokenStartLine) +
                                    ": unterminated string quote.";
                                return result;
                            }

                            auto isHex = [](char ch) {
                                return (ch >= '0' && ch <= '9') ||
                                    (ch >= 'A' && ch <= 'F') ||
                                    (ch >= 'a' && ch <= 'f');
                            };

                            if (!isHex(str[i])) {
                                result.error = true;
                                result.errorLine = tokenStartLine;

                                return result;
                            }

                            escapeSequence.push_back(str[i]);
                            i++;


                            if (i < str.size() && isHex(str[i])) {
                                escapeSequence.push_back(str[i]);
                                i++;
                            }

                            emit("ESCAPED_CHARACTER", escapeSequence, tokenStartLine, result);
                            continue;
                        }
                        else {
                            result.error = true;
                            result.errorLine = tokenStartLine;
                            return result;
                        }
                    }

                    currentToken.push_back(str[i]);
                    i++;
                }

                if (i >= str.size()) {
                    result.error = true;
                    result.errorLine = tokenStartLine;
                    result.errorMessage =
                        "Syntax error on line " + to_string(tokenStartLine) +
                        ": unterminated string quote.";
                    return result;
                }

                if (!currentToken.empty()) {
                    emit("STRING", currentToken, tokenStartLine, result);
                    currentToken.clear();
                }
                emit("DOUBLE_QUOTE", "\"", lineNumber, result);
                state = 0;
                i++;
                break;
            }
    }
    if (state == 1) {
    emit("IDENTIFIER", currentToken, tokenStartLine, result);
    }
    else if (state == 2) {
        emit("INTEGER", currentToken, tokenStartLine, result);
    }
    else if (state == 3 || state == 4) {
        result.error = true;
        result.errorLine = tokenStartLine;
    }
    emit("EOF", "EOF", lineNumber, result);
    return result;
}