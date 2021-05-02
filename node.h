// Author: Austin Wildgrube <akwwb6@mail.umsl.edu>
// Date: 04/04/2021

#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <utility>

/**
 * Holds our node information
 */
struct Node {
    // An array to hold multiple tokens on the node
    Token token;

    // Node children
    struct Node *childOne, *childTwo, *childThree, *childFour;

    int counter;
    int level;
    int lineNumber;

    explicit Node(const Token& token) {
        this->token = token;
        this->counter = 0;
        this->level = 0;
        this->lineNumber = 0;
        this->childOne = this->childTwo = this->childThree = this->childFour = nullptr;
    }
};

#endif //NODE_H
