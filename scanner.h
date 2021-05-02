// Author: Austin Wildgrube <akwwb6@mail.umsl.edu>
// Date: 03/08/2021

#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include "token.h"

class Scanner {
public:
    static Token scan(FILE *, char, char);
private:
    static int getColumn(char);
    static int searchTokens(int, int);
    static bool checkKeywords(const std::string&);
    static Token getPrintStatement(int, const std::string&, int);
    static void getErrorStatement(const std::string&, int);
};

#endif //SCANNER_H