#ifndef IGNORECOMMENTS_H
#define IGNORECOMMENTS_H

#include <string>

struct CommentResult {
    bool unterminated;
    int unterminatedLine;
    std::string text;  
};

CommentResult ignoreComments(std::string src);

#endif // IGNORECOMMENTS_H