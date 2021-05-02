// Author: Austin Wildgrube <akwwb6@mail.umsl.edu>
// Date: 03/08/2021

#ifndef TOKEN_H
#define TOKEN_H

extern std::string keywordArray[20];
extern std::string tokenID[];
extern std::string tokenName[];

// Our Token struct is what we print from
struct Token {
    std::string id;
    std::string name;
    std::string userInput;
    std::string block;
    int successId = 0;
    int lineNumber = 0;
};

#endif //TOKEN_H
