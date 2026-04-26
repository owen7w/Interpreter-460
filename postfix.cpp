#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <stack>
#include <cmath>
#include "token.h"

using namespace std;

int precedence(const token& curr){
    if (curr.type == "BOOLEAN_NOT") return 6;        
    if (curr.type == "CARET") return 5;
    if (curr.type == "ASTERISK" || curr.type == "DIVIDE" || curr.type == "MODULO") return 4;
    if (curr.type == "PLUS" || curr.type == "MINUS") return 3;
    if (curr.type == "LT" || curr.type == "LT_EQUAL" ||
        curr.type == "GT" || curr.type == "GT_EQUAL" ||
        curr.type == "BOOLEAN_EQUAL" || curr.type == "BOOLEAN_NOT_EQUAL") return 2;
    if (curr.type == "BOOLEAN_AND") return 1;
    if (curr.type == "BOOLEAN_OR") return 0;
    return -1;
}

bool isOperator(token& curr){
    if (curr.type == "MINUS" || curr.type == "PLUS" || curr.type == "DIVIDE" || curr.type == "ASTERISK" || curr.type == "MODULO"
    || curr.type == "GT" || curr.type == "LT" || curr.type == "BOOLEAN_NOT" || curr.type == "BOOLEAN_NOT_EQUAL" 
    || curr.type == "BOOLEAN_AND" || curr.type == "BOOLEAN_OR" || curr.type == "LT_EQUAL"
    || curr.type == "GT_EQUAL" || curr.type == "BOOLEAN_EQUAL" || curr.type == "CARET" )
        return 1;
    return 0;
}

int floorDiv(int a, int b) {
    if (a * b < 0 && a % b != 0)
        return (a / b) - 1;
    return a / b;
}

// Calculate from Postfix notation:
int solvePost(vector<token>& output){
    stack<int> stack;

    for (token curr : output) {
        
        if (curr.type == "IDENTIFIER"){
            // #### CHANGE ME #####
            // get identifer value from identifier list
            int changeMe = -99;
            stack.push(changeMe); // the value from getting identifier value
        }
        else if (curr.type == "INTEGER"){
            stack.push(stoi(curr.text));
        }
        // boolean not only affects 1 token, while the others affect 2, so handle seperately
        else if (curr.type == "BOOLEAN_NOT"){
            int val = stack.top(); stack.pop();
            stack.push(!val);
        }
        // Otherwise, it must be an operator
        else {
            int val1 = stack.top(); stack.pop(); 
            int val2 = stack.top(); stack.pop(); 
            if (curr.type == "PLUS") stack.push(val2 + val1);
            else if (curr.type == "MINUS") stack.push(val2 - val1);
            else if (curr.type == "ASTERISK") stack.push(val2 * val1);
            else if (curr.type == "DIVIDE") {
                if (val1 == 0) {
                    cerr << "Error: division by zero" << endl;
                    return -1;
                }
                stack.push(floorDiv(val2, val1));}
            else if (curr.type == "MODULO") {
                if (val1 == 0) {
                    cerr << "Error: division by zero" << endl;
                    return -1;
                }
                stack.push(val2 % val1);}
            else if (curr.type == "CARET") stack.push((int)pow(val2, val1));
            else if (curr.type == "BOOLEAN_OR") stack.push(val2 || val1);
            else if (curr.type == "BOOLEAN_AND") stack.push(val2 && val1);
            else if (curr.type == "GT") stack.push(val2 > val1);
            else if (curr.type == "LT") stack.push(val2 < val1);
            else if (curr.type == "GT_EQUAL") stack.push(val2 >= val1);
            else if (curr.type == "LT_EQUAL") stack.push(val2 <= val1);
            else if (curr.type == "BOOLEAN_EQUAL") stack.push(val2 == val1);
            else if (curr.type == "BOOLEAN_NOT_EQUAL") stack.push(val2 != val1);
        }
    }
    return stack.top();
}
//Parses the list into postfix order
int postfix(vector<token>& list, vector<token>& output){
    stack<token> stack;
    for (token curr : list){
        //if operand, append to output
        if (curr.type == "IDENTIFIER" || curr.type == "INTEGER"){
            output.push_back(curr);
        }
        // if left parenthesis, stack
        else if (curr.type == "L_PAREN"){
            stack.push(curr);
        }
        // if right parenthesis, pop stack until find left parenthesis (remove left and right parenth)
        else if (curr.type == "R_PAREN"){
            while (!stack.empty() && stack.top().type != "L_PAREN"){
                output.push_back(stack.top());
                stack.pop();
            }
            if (!stack.empty()) stack.pop(); // remove (
        }
        // if * / + -, first remove any higher or equal precedence, then push to stack.
        else if (isOperator(curr)) {
            while (!stack.empty() && (stack.top().type != "L_PAREN") &&
            ((precedence(curr) < precedence(stack.top())) ||
            (precedence(curr) == precedence(stack.top()) && curr.type != "CARET"))) {
                output.push_back(stack.top());
                stack.pop();
            }
            stack.push(curr);
        }
    }
    // final stack flush
    while (!stack.empty()) {
        if (stack.top().type == "L_PAREN" || stack.top().type == "R_PAREN") {
            cerr << "Error: missing parenthesis" << endl;
            return 0; 
        }
    output.push_back(stack.top());
    stack.pop();
    }
    return solvePost(output);
}


int main(){
    string input = "text.txt";
    ifstream file(input);
    if (!file) {cerr << "cannot open input file: " << input << "\n"; 
        return 1;}
    ofstream result("output", ios::out);
    
    // #### CHANGE ME #####
    // get list of tokens into list
    vector<token> list;
    vector<token> output;

    cout << postfix(list, output) << endl;
    
    return 0;
}


