#include <iostream>
#include <fstream>
#include <cstring>
#include "include/token.h"
#include <vector>

using namespace std;

// HW 2:

vector<token> Tokenizer(istream &file, ostream &result) {    

    vector<token> tokens;
    char ch;
    int state = 0;
    bool doubleQuoted = false;
    bool singleQuoted = false;
    token currentToken = token();
    result << "Token list:" << endl << endl;
    while(file.get(ch)){
        
        
        switch(state){
            
            //Default state:    
            case 0:
                currentToken.type = "CHARACTER";

                if ((ch == ' ')){
                        break;
                    }
                
                // check for escape character case
                else if (ch == '\\'){
                    currentToken.token += ch;
                    state = 2;
                    break;
                }

                // string parser
                else if (doubleQuoted){
                    currentToken.token += ch;
                    state = 11;
                    break;
                }
                else if (singleQuoted){
                    currentToken.token += ch;
                    state = 11;
                    break;
                }
               
                // !!   is a quotation 
                else if (ch == '"'){
                    currentToken.type = "DOUBLE_QUOTE"; 
                    doubleQuoted = true;
                    currentToken.token += ch;
                    state = 100;
                    break;
                    
                }

                else if (ch == '\''){
                    currentToken.type = "SINGLE_QUOTE";
                    singleQuoted = true;
                    currentToken.token += ch;
                    state = 100;
                    break;
                }
                
                else if (ch == '+'){
                    currentToken.token += ch;
                    currentToken.type = "PLUS"; 
                    state = 3;
                    break;
                }
                else if (ch == '-'){
                    currentToken.token += ch;
                    currentToken.type = "MINUS"; 
                    state = 3;
                    break;
                }
                else if ((ch >= '0') && (ch <= '9')){
                    currentToken.token += ch;
                    currentToken.type = "WHOLE_NUMBER";
                    state = 3;
                    break;
                }

                 // start identifier
                else if (((ch >= 'A') && (ch <= 'Z')) || ((ch >= 'a') && (ch <= 'z')) || (ch == '_') || ((ch >= '0') && (ch <= '9')) ){
                    currentToken.type = "CHARACTER";
                    currentToken.token += ch;
                    state = 1;
                    //cout << ch << " ";
                    break;
                }
              
                else if (ch == '*'){
                    currentToken.type = "ASTERISK"; 
                    currentToken.token += ch;
                    state = 100;
                    break;
                }
                // is a parentheses
                else if (ch == '('){
                    currentToken.type = "L_PAREN"; 
                    currentToken.token += ch;
                    state = 100;
                    break;
                }
                else if (ch == ')'){
                    currentToken.type = "R_PAREN";    
                    currentToken.token += ch;
                    state = 100;
                    break; 
                }

                    // is a bracket
                else if (ch == '{'){
                    currentToken.type = "L_BRACE"; 
                    currentToken.token += ch;
                    state = 100;
                    break;
                }
                else if (ch == '}'){
                    currentToken.type = "R_BRACE"; 
                    currentToken.token += ch;
                    state = 100;
                    break;
                }

                    // is a square bracket
                else if (ch == '['){
                    currentToken.type = "L_BRACKET"; 
                    currentToken.token += ch;
                    state = 100;
                    break;
                }
                else if (ch == ']'){
                    currentToken.type = "R_BRACKET"; 
                    currentToken.token += ch;
                    state = 100;
                    break;
                }

                

                // is a semicolon/comma
                else if (ch == ';'){
                    currentToken.type = "SEMICOLON"; 
                    currentToken.token += ch;
                    state = 100;
                    break;
                }
                else if (ch == ','){
                    currentToken.type = "COMMA"; 
                    currentToken.token += ch;
                    state = 100;
                    break;
                }

                // is a operator
                else if (ch == '/'){
                    currentToken.type = "DIVIDE"; 
                    currentToken.token += ch;
                    state = 100;
                    break;
                }
                else if (ch == '%'){
                    currentToken.type = "MODULO"; 
                    currentToken.token += ch;
                    state = 100;
                    break;
                }
                else if (ch == '^'){
                    currentToken.type = "CARET"; 
                    currentToken.token += ch;
                    state = 100;
                    break;
                }
               
                // needs more states
                else if (ch == '=' && file.peek() == '='){
                    currentToken.type = "BOOLEAN_EQUAL"; 
                    currentToken.token += '=';
                    file.get();
                    currentToken.token += ch;
                    state = 100;
                    break;
                }
                else if (ch == '!' && file.peek() == '='){
                    currentToken.type = "BOOLEAN_NOT_EQUAL"; 
                    currentToken.token += '!';
                    file.get();
                    currentToken.token += ch;
                    state = 100;
                    break;
                }
                else if (ch == '<' && file.peek() == '='){
                    currentToken.type = "LT_EQUAL"; 
                    currentToken.token += '<';
                    file.get();
                    currentToken.token += ch;
                    state = 100;
                    break;
                }
                else if (ch == '>' && file.peek() == '='){
                    currentToken.type = "GT_EQUAL"; 
                    currentToken.token += '>';
                    file.get();
                    currentToken.token += ch;
                    state = 100;
                    break;
                }
                else if (ch == '&' && file.peek() == '&'){
                    currentToken.type = "BOOLEAN_AND"; 
                    currentToken.token += '&';
                    file.get();
                    currentToken.token += ch;
                    state = 100;
                    break;
                }
                else if (ch == '|' && file.peek() == '|'){
                    currentToken.type = "BOOLEAN_OR"; 
                    currentToken.token += '|';
                    file.get();
                    currentToken.token += ch;
                    state = 100;
                    break;
                }
                
                // equal since bool equal above
                else if (ch == '='){
                    currentToken.type = "ASSIGNMENT_OPERATOR"; 
                    currentToken.token += ch;
                    state = 100;
                    break;
                }
                // comparison
                else if (ch == '<'){
                    currentToken.type = "LT"; 
                    currentToken.token += ch;
                    state = 100;
                    break;
                }
                else if (ch == '>'){
                    currentToken.type = "GT"; 
                    currentToken.token += ch;
                    state = 100;
                    break;
                }
                //Booleans
                else if (ch == '!'){
                    currentToken.type = "BOOLEAN_NOT"; 
                    currentToken.token += ch;
                    state = 100;
                    break;
                }
                else {
                    break;
                }
                break;
                
            // handle identifier 
            case 1:
                
                if (((ch >= 'A') && (ch <= 'Z')) || ((ch >= 'a') && (ch <= 'z')) || (ch == '_')){
                    currentToken.type = "IDENTIFIER";
                    currentToken.token += ch;
                    //cout << ch << " ";
                }
                
                else {
                    state = 100; 
                    
                    break;
                }
                break;
            
            // handle quotations:
            case 11:

                if (ch == '\\'){
                    currentToken.token += ch;
                    state = 2;
                }

                else if(tokens.back().type == "DOUBLE_QUOTE") {
                     
                    file.unget();
                    state = 12;
                }
                else {
                    file.unget();
                    state = 13;
                }
                break;
                
            // handle escape char => "\"

            case 12:
                currentToken.type = "STRING"; 
                currentToken.token += ch;
                while (file.get(ch)){
                    
                    if (ch == '"') {
                        doubleQuoted = false;
                        cout << "Token type: " << currentToken.type << endl << "Token: " << currentToken.token << " doubleQuote?:" << doubleQuoted <<endl << endl;

                        if (!currentToken.token.empty()) {
                            tokens.push_back(currentToken);
                            }
                        currentToken.token = '"';
                        currentToken.type = "DOUBLE_QUOTE";
                        tokens.push_back(currentToken);
                        currentToken = token();
                        state = 0;
                        break;
                    }
                    else{
                        currentToken.token += ch;
                    }
                        //supposed toend
                    if (ch == '\n'){
                        cout << "error";
                        break;
                    }
                }
                break;
                // single
            case 13:
                currentToken.type = "STRING"; 
                currentToken.token += ch;
                while (file.get(ch)){
                    
                    if ((ch == '\'')){
                        singleQuoted = false;
                        cout << "Token type: " << currentToken.type << endl << "Token: " << currentToken.token << " doubleQuote?:" << doubleQuoted <<endl << endl;

                        if (!currentToken.token.empty()) {
                            tokens.push_back(currentToken);
                            }
                        currentToken.token = '\'';
                        currentToken.type = "SINGLE_QUOTE";
                        tokens.push_back(currentToken);
                        currentToken = token();
                        
                        state = 0;
                        break;
                    } 
                    else{
                        currentToken.token += ch;
                    }
                        //supposed toend
                    if (ch == '\n'){
                        cout << "error";
                        break;
                    }
                }
                break;
               
            case 2:
                currentToken.type = "ESCAPED_CHARACTER"; 
                if (ch == 'x'){
                    currentToken.token += ch;
                    state = 20;
                }
                
                else if (ch == 'a' || ch == 'b' || ch == 'f' || ch == 'n' ||
                ch == 'r' || ch == 't' || ch == 'v' || ch == '\\' || ch == '?' ||
                ch == '\'' || ch == '"'){
                    currentToken.token += ch;
                }
                
                // if prev is string, append to it
                if (tokens.back().type == "STRING"){
                    tokens.back().token = tokens.back().token + currentToken.token;
                    currentToken = token();
                    state = 0;
                    break;
                } 
                
                else {
                    state = 100;
                    
                    break;}
                break;
            // handle integers
            case 3:
                currentToken.type = "INTEGER";
                if ((ch >= '0') && (ch <= '9')){
                    currentToken.token += ch;
                }
                else if ((ch == '*') || (ch == ' ') || (ch == ';') || (ch == '\n')|| (ch == ')') ){
                    
                    state = 100;
                    break;
                }
                else{
                    cout << "invalid integer" << endl;
                    state = 101;
                    break;
                }
                break;
            
            //quotations - start string detecting
            case 4: 
                cout << currentToken.token << " " << currentToken.type << endl;

                if (!currentToken.token.empty()) {
                   tokens.push_back(currentToken);
                }
                state = 5;
                break;
            
            
            // hex digit loop
            case 20:
                // is hex digit?
                
                if (((tolower(ch) >= 'a') && (tolower(ch) <= 'f')) || 
                ((ch >= '0') && (ch <= '9'))){
                    currentToken.token += ch;
                }
                else {
                    state = 100;
                }
                break;

            // create it into a token and prepare for next token
            case 100:
                
                file.unget();
                
                cout << "Token type: " << currentToken.type << endl << "Token: " << currentToken.token << " doubleQuote?:" << doubleQuoted <<endl << endl;

                if (!currentToken.token.empty()) {
                   tokens.push_back(currentToken);
                }
                state = 0;
                currentToken = token();
                break;
                           
        }
        
    }
     for (int i = 0; i < tokens.size(); i++) { 
        result << "Token type: " << tokens.at(i).type << endl;
        result << "Token: \t" << tokens.at(i).token << endl <<
        endl;

    }
    return tokens;

}


