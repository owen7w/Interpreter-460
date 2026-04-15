#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include "include/remove_comments.h"
#include "include/tokenizer.h"
#include "include/decentParser.h"

using namespace std;

string input;

int main() {
    //cout << "Put in name of input file: " ;
    //cin >> input;
    input = "hw2/test_file.c";
    ifstream file(input);
    if (!file) cerr << "cannot open input file: " << strerror(errno) << "\n"; 
 
    
    stringstream filestring;
    removeComments(file, filestring);
    cout << "successfully removed comments" << endl;

    ofstream result(input.substr(0, input.size()-2) + "-tokenized", ios::out);

    vector<token> list = Tokenizer(filestring, result);
    decentParser(list);
    
    result.close();

}