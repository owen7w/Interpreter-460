#include <iostream>
#include <string>

using namespace std;

static char decodeCharLiteral(const string& text)
{
    if (text == "\\n") return '\n';
    if (text == "\\t") return '\t';
    if (text == "\\0") return '\0';
    if (text.size() >= 2 && text[0] == '\\' && text[1] == 'x')
    {
        string hexStr = text.substr(2);
        if (hexStr.empty()) return '\0';
        return (char)stoi(hexStr, nullptr, 16);
    }
    if (text.empty()) return '\0';
    if (text[0] == '\\' && text.size() >= 2) return text[1];
    return text[0];
}

int main() {
    string t = "\\x0";
    char c = decodeCharLiteral(t);
    cout << "Char value: " << (int)c << endl;
    return 0;
}
