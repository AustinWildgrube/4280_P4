// Author: Austin Wildgrube <akwwb6@mail.umsl.edu>
// Date: 04/04/2021

#ifndef PARSER_H
#define PARSER_H

#include "token.h"

class Parser {
public:
    static void parser(const char* fileName, bool, const std::string&);
private:
    static struct Node *addStructure(struct Node *, const Token&, struct Node *, std::string);
    static void printPreorder(struct Node*);
    static void traverseTree(struct Node*);

    static std::string newTemp();
    static std::string newTempLabel();
    static std::string* splitUserInput(struct Node*);
    static void checkVariables(struct Node*, std::string*);

    static void insertVar(std::string, int, int, int);
    static void popVar();
    static void searchVar(const std::string&, int);
    static Token getNewToken();

    static struct Node *programToken(struct Node *);
    static struct Node *varsToken(struct Node *, struct Node *);
    static struct Node *exprToken(struct Node *, struct Node *);
    static struct Node *nToken(struct Node *, struct Node *);
    static struct Node *aToken(struct Node *, struct Node *);
    static struct Node *mToken(struct Node *, struct Node *);
    static struct Node *rToken(struct Node *, struct Node *);
    static struct Node *blockToken(struct Node *, struct Node *);
    static struct Node *statsToken(struct Node *, struct Node *);
    static struct Node *statToken(struct Node *, struct Node *);
    static struct Node *mStatToken(struct Node *, struct Node *);
    static struct Node *inToken(struct Node *, struct Node *);
    static struct Node *outToken(struct Node *, struct Node *);
    static struct Node *ifToken(struct Node *, struct Node *);
    static struct Node *loopToken(struct Node *, struct Node *);
    static struct Node *assignToken(struct Node *, struct Node *);
    static struct Node *r0Token(struct Node *, struct Node *);
    static struct Node *labelToken(struct Node *, struct Node *);
    static struct Node *gotoToken(struct Node *, struct Node *);

};

struct varStack_t {
    std::string name;
    int lineNumber = 0;
    bool isGlobal = true;
    int blockCount = 0;
};

struct blockStack_t {
    int varsInBlock = 0;
    int blockLevel = 0;
};

struct vars_t {
    std::string name;
    int value = 0;
};

#endif //PARSER_H
