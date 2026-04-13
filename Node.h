#ifndef NODE_H
#define NODE_H

#include <string>
using namespace std;

struct Node {
    string label;    
    string text;   
    int line;

    Node* child;     
    Node* sibling;   

    Node(const string& lbl = "", const string& txt = "", int ln = 0)
        : label(lbl), text(txt), line(ln), child(nullptr), sibling(nullptr) {}
};

#endif