#include "Node.h"

Node* makeNode(const std::string& label, int line) {
    return new Node(label, line);
}

void addChild(Node* parent, Node* child) {
    if (parent == nullptr || child == nullptr) {
        return;
    }

    if (parent->leftChild == nullptr) {
        parent->leftChild = child;
        return;
    }

    Node* cur = parent->leftChild;
    while (cur->rightSibling != nullptr) {
        cur = cur->rightSibling;
    }

    cur->rightSibling = child;
}

