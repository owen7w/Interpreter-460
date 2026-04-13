//Mario Lucido Ignore comments in a file and output the result to a new file
#include <iostream>
#include <fstream>
#include <string>
#include <sstream> 
using namespace std;   
#include "ignoreComments.h"



CommentResult ignoreComments(std::string str) {
    int state = 0; 
    int line =1; 
    int slashLine = 1;
    int blockStartLine = 0; 
    int beginComment = 0;
    for(size_t i=0; i<str.size(); i++){
        char c = str[i];

        if(c == '\n') line++;

        switch(state){
            case 0:
                if(c == '/'){
                    beginComment = i;
                    slashLine = line;
                    state = 1; 
                } else if (c == '"') {
                    state = 5; 
                }
                if (c == '*' && i + 1 < str.size() && str[i + 1] == '/') {
                    CommentResult r;
                    r.unterminated = true;
                    r.unterminatedLine = line;
                    r.text = str;
                    return r;
                }
                break;
            case 1:
                if(c == '*'){
                    state = 2; 
                    blockStartLine = slashLine;
                    str[beginComment] = ' ';
                    str[i] = ' ';
                } else if (c == '/') {
                    str[beginComment] = ' ';
                    str[i] = ' ';
                    state = 4; 
                } else {
                    state = 0; 
                }
                break;
            case 2:
                if(c != '\n') {
                    str[i] = ' ';
                }
                if(c == '*'){
                    state = 3; 
                }
                break;
            case 3:
                if(c != '\n') {
                    str[i] = ' ';
                }
                if(c == '/') {
                    state = 0; 
                } else if (c != '*') {
                    state = 2; 
                }
                break;
            case 4:
                if(c != '\n') {
                    str[i] = ' ';
                } else {
                    state = 0; 
                }
                break;
            case 5:
                if(c == '\\') {
                    state = 6; 
                } else if (c == '"') {
                    state = 0; 
                }
                break;
            case 6:
                state = 5; 
                break;
        }
    }
    CommentResult r;
    r.unterminated = (state == 2 || state == 3);
    r.unterminatedLine = blockStartLine;
    r.text = str;
    return r;
}





















/*


int main(int argc, const char** argv) {
    
    // Check if the correct number of arguments is provided
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }
    // Open the input file
    ifstream input_file(argv[1]);
    if (!input_file.is_open()) {
        cout << "Error opening file: " << argv[1] << std::endl;
        return 1;
    }
    // Read the entire file into a string
    string str((istreambuf_iterator<char>(input_file)), istreambuf_iterator<char>());
    input_file.close();

    // states 0 = normal code 1 = potential comment start 2 = block comment 3 = potential block comment end 4 = line comment 5 = string literal 6 = escape sequence in string literal 
        if (state == 2 || state == 3) {
        cout << "ERROR: Program contains C-style, unterminated comment on line " << blockStartLine << endl;
    }
    size_t dot = string(argv[1]).find_last_of(".");
    string output_filename = string(argv[1]).substr(0, dot) + "-comments_replaced_with_whitespace.c";
    ofstream output_file(output_filename);
    if (!output_file){
        cout  << "Error creating output file: " << output_filename << endl;
        return 1;
    }
    output_file << str;
    output_file.close();
    return 0;
}
    */