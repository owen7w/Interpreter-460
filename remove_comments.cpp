#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

// HW 1:

int removeComments(ifstream &file, ostream &result) {  
    char ch;
    int state = 0;
    int line = 1;
    bool commenting = false;

    while(file.get(ch)){
        //Line number:
        if (ch == '\n')
            line += 1;
        
        switch(state){
            //Default State
            case 0:
                if (ch == '/'){
                    state = 1;
                }
                else if (ch == '"'){ 
                    result << ch;
                    state = 5;
                }
               else if ((ch == '*') && (commenting == true)){ 
                    result << ch;
                    state = 6;
                }
                if (state == 0){
                    result << ch;
                }
                break;

            //Handle Comment type distinction
            case 1:
                if (ch == '/'){
                    state = 2;
                }
                //block comment
                else if (ch == '*'){
                    state = 3;
                }
                else {
                    //needed a slash to enter this state, we know its not a comment so we put it back into our result iostream
                    result << '/' << ch;
                    state = 0;
                }
                break;

            //Handle Line comments
            case 2:
                while(file.get(ch)){
                    if (ch == '\n'){
                        result << "\n";
                    line += 1;
                    state = 0;
                    break;} 
                }
                break;
 
            //Handle Block Comments 
            case 3:
                //Check Current ch before iterating
                if (ch == '*'){
                    commenting = true;
                    state = 4;
                    break;
                }
                //Iterate till we find a star
                while(file.get(ch)){
                    if (ch == '*'){
                        state = 4;
                        break;
                    }
                    if (ch == '\n'){
                        result << '\n';
                        line += 1;
                    }    
                } 
                // error:
                cout << "ERROR: Program contains C-style, unterminated comment on line " << line << endl;
                return 1;

            //Close Block Comment
            case 4:
                while (ch == '*'){
                    file.get(ch);
                }
                if (ch == '\n'){
                    result << '\n';
                    line += 1;
                }
                if (ch == '/'){
                    commenting = true;
                    state = 0;
                    break;
                }
                break;

            //Handle comments
            case 5:
                result << ch;
                while(file.get(ch)){
                    result << ch;
                    if (ch == '"'){
                        state = 0;
                        break;
                    }  
                }
                break;
            case 6: 
                if (ch == '/'){ 
                    cout << endl << "ERROR: Program contains C-style, unterminated comment on line " << line << endl;
                    return 1;
                }
                break;
        }  
            
    }
    file.close();
    return 1;
}

