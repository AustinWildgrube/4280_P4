// Author: Austin Wildgrube <akwwb6@mail.umsl.edu>
// Date: 05/04/2021

#include <iostream>
#include <cstdio>
#include <utility>
#include <fstream>
#include <sstream>
#include <vector>

#include "parser.h"
#include "scanner.h"
#include "node.h"

using namespace std;

FILE *file;
Token globalToken;
ofstream asmFile;

struct varStack_t varStack[99];
struct vars_t vars[99];
struct blockStack_t blockStack[99];
char character, lookAhead;
bool isGlobal = true;
int varTotal;
int blockCount;
int varCt = 0;
int labelCt = 0;

/**
 * Main parse function
 * @param fileName
 */
void Parser::parser(const char* fileName, bool isKeyboard, const string& usersFile) {
    struct Node* root = nullptr;

    // Open our file
    file = fopen(fileName, "r");

    // Create our ASM file
    if (isKeyboard)
        asmFile.open("kb.asm");
    else {
        string usersFileConcat = usersFile + ".asm";
        asmFile.open(usersFileConcat);
    }

    // We need to insert our root node ahead of time so it won't be inserted twice.
    Token fakeToken;
    fakeToken.block = "<program>";
    fakeToken.userInput = "main";
    fakeToken.lineNumber = globalToken.lineNumber;
    root = Parser::addStructure(root, fakeToken, nullptr, "");

    // Get our first token
    globalToken = Parser::getNewToken();

    // Start parsing
    Parser::programToken(root);

    // Traverse our parse tree
    Parser::traverseTree(root);

    // We are done so print STOP
    asmFile << "STOP\n";

    // We finish by printing our vars
    for (auto & var : vars) {
        if (!var.name.empty()) {
            asmFile << var.name << " " << var.value << "\n";
        }
    }

    // Close our file
    asmFile.close();

//    Parser::printPreorder(root);
}

/**
 * We can add our nodes to the tree here, but we can also edit existing nodes in the case
 * that we have an operator that was discovered after the fact.
 * @param node
 * @param token
 * @param originNode
 * @param userInput
 * @return
 */
struct Node *Parser::addStructure(struct Node *node, const Token& token, struct Node *originNode,
    string userInput = "") {

    // If the user input is not empty we simply just edit the node
    // instead of adding one
    if (!userInput.empty()) {
        node->token.userInput = std::move(userInput);
    }

    // If there is no node then create one
    if (node == nullptr) {
        Node *newNode = new Node(token);

        // We can find which level it needs to be by looking at the root nodes
        // level and adding one.
        if (originNode == nullptr) {
            newNode->level = 0;
        } else {
            newNode->level = originNode->level + 1;
        }

        newNode->lineNumber = token.lineNumber;

        return newNode;
    }

    return node;
}

/**
 * Here we print our tree in preorder fashion
 * @param node
 */
void Parser::printPreorder(struct Node* node) {
    // Check to make sure tree isn't empty
    if (node == nullptr)
        return;

    // Count our spaces
    for (int j = 0; j < node->level; j++) {
        cout << "  ";
    }

    // After printing the spaces we then print the leaf info
    cout << node->token.block << " " << node->token.userInput << endl;

    // Recursively process children
    Parser::printPreorder(node->childOne);
    Parser::printPreorder(node->childTwo);
    Parser::printPreorder(node->childThree);
    Parser::printPreorder(node->childFour);
}

/**
 * Here we traverse our parse tree in preorder fashion and process the userInput.
 * @param node
 */
void Parser::traverseTree(struct Node* node) {
    auto *variableIdentifiers = (string *) "";

    // Check to make sure tree isn't empty
    if (node == nullptr)
        return;

    // Since our token.userInput is concatenated words we need to split them apart
    if (!node->token.userInput.empty()) {
        // Get out user input
        variableIdentifiers = Parser::splitUserInput(node);

        // Check the variables in the statement
        Parser::checkVariables(node, variableIdentifiers);
    }

    // Convert to our ASM code
    if (node->token.block == "<goto>") {
        asmFile << "BR " << variableIdentifiers[1] << "\n";
    } else if (node->token.block == "<label>") {
        asmFile << variableIdentifiers[1] << ": NOOP \n";
    } else if (node->token.block == "<if>") {
        Parser::traverseTree(node->childTwo);
        string RO = variableIdentifiers[0];
        string R1 = variableIdentifiers[1];
        string R2 = variableIdentifiers[2];

        Parser::traverseTree(node->childThree);
        string tempVar = newTemp();

        asmFile << "STORE " << tempVar << "\n";
        Parser::traverseTree(node->childOne);

        string Label = newTempLabel();
        if (RO == "=>") {
            asmFile << "SUB " << tempVar << "\n";
            asmFile << "BRPOS " << Label << "\n";
        } else if (RO == "=<") {
            asmFile << "SUB " << tempVar << "\n";
            asmFile << "BRNEG " << Label << "\n";
        } else if (RO == "==") {
            asmFile << "SUB " << tempVar << "\n";
            asmFile << "BRPOS " << Label << "\n";
            asmFile << "BRNEG " << Label << "\n";
        } else if (RO == "[" && R1 == "==" && R2 == "]") {
            asmFile << "SUB " << tempVar << "\n";
            asmFile << "BRZERO " << Label << "\n";
        } else if (RO == "%") {
            asmFile << "MULT " << tempVar << "\n";
            asmFile << "BRPOS " << Label << "\n";
        }

        Parser::traverseTree(node->childFour);
        asmFile << Label << ": NOOP\n";
    } else if (node->token.block == "<loop>") {
        Parser::traverseTree(node->childTwo);
        string RO = variableIdentifiers[0];
        string R1 = variableIdentifiers[1];
        string R2 = variableIdentifiers[2];

        string tempVar = newTemp();
        string startLabel = newTempLabel();
        string endLabel = newTempLabel();

        asmFile << startLabel << ": ";
        Parser::traverseTree(node->childOne);
        asmFile << "STORE " << tempVar << "\n";
        Parser::traverseTree(node->childThree);

        if (RO == "=>") {
            asmFile << "SUB " << tempVar << "\n";
            asmFile << "BRPOS " << endLabel << "\n";
        } else if (RO == "=<") {
            asmFile << "SUB " << tempVar << "\n";
            asmFile << "BRNEG " << endLabel << "\n";
        } else if (RO == "==") {
            asmFile << "SUB " << tempVar << "\n";
            asmFile << "BRPOS " << endLabel << "\n";
            asmFile << "BRNEG " << endLabel << "\n";
        } else if (RO == "[" && R1 == "==" && R2 == "]") {
            asmFile << "SUB " << tempVar << "\n";
            asmFile << "BRZERO " << endLabel << "\n";
        } else if (RO == "%") {
            asmFile << "MULT " << tempVar << "\n";
            asmFile << "BRPOS " << endLabel << "\n";
        }

        Parser::traverseTree(node->childFour);
        asmFile << "BR " << startLabel << "\n";
        asmFile << endLabel << ": NOOP\n";
    } else if (node->token.block == "<assign>") {
        string varName = variableIdentifiers[1];

        Parser::traverseTree(node->childOne);
        asmFile << "STORE " << varName << "\n";
    } else if (node->token.block == "<in>") {
        asmFile << "READ " << variableIdentifiers[1] << "\n";
    } else if (node->token.block == "<out>") {
        string tempVar = newTemp();

        Parser::traverseTree(node->childOne);
        asmFile << "STORE " << tempVar << "\n";
        asmFile << "WRITE " << tempVar << "\n";
    } else if (node->token.block == "<expr>") {
        if (variableIdentifiers[0] == "-") {
            string tempVar = Parser::newTemp();

            Parser::traverseTree(node->childTwo);
            asmFile << "STORE " << tempVar << "\n";

            Parser::traverseTree(node->childOne);
            asmFile << "SUB " << tempVar << "\n";
        } else {
            Parser::traverseTree(node->childOne);
        }
    } else if (node->token.block == "<N>") {
        if (variableIdentifiers[0] == "/") {
            string tempVar = Parser::newTemp();

            Parser::traverseTree(node->childTwo);
            asmFile << "STORE " << tempVar << "\n";

            Parser::traverseTree(node->childOne);
            asmFile << "DIV " << tempVar << "\n";

        } else if (variableIdentifiers[0] == "*") {
            string tempVar = Parser::newTemp();

            Parser::traverseTree(node->childTwo);
            asmFile << "STORE " << tempVar << "\n";

            Parser::traverseTree(node->childOne);
            asmFile << "MULT " << tempVar << "\n";
        } else {
            Parser::traverseTree(node->childOne);
        }
    } else if (node->token.block == "<A>") {
        if (variableIdentifiers[0] == "+") {
            string tempVar = Parser::newTemp();

            Parser::traverseTree(node->childTwo);
            asmFile << "STORE " << tempVar << "\n";

            Parser::traverseTree(node->childOne);
            asmFile << "ADD " << tempVar << "\n";
        } else {
            Parser::traverseTree(node->childOne);
        }
    } else if (node->token.block == "<M>") {
        if (variableIdentifiers[0] == "*") {
            Parser::traverseTree(node->childOne);
            asmFile << "MULT -1\n";
        } else {
            Parser::traverseTree(node->childOne);
        }
    } else if (node->token.block == "<R>") {
        asmFile << "LOAD " << variableIdentifiers[0] << "\n";
    } else {
        // Recursively process children
        Parser::traverseTree(node->childOne);
        Parser::traverseTree(node->childTwo);
        Parser::traverseTree(node->childThree);
        Parser::traverseTree(node->childFour);
    }
}

/**
 * Here where we check if our token is a variable or if it uses a variable. If it uses a variable, we need to
 * search and if it is a variable we need to add it to our array
 * @param node
 * @param variableIdentifiers
 */
void Parser::checkVariables(struct Node* node, string* variableIdentifiers) {
    // Here we check level to see what block we are in and if we need to pop
    if (node->level <= blockStack[blockCount].blockLevel && blockCount != 0) {
        // Pop block because we found the end
        Parser::popVar();
    }

    // Here we are beginning a new block
    if (node->token.block == "<block>") {
        blockCount++;
        blockStack[blockCount].blockLevel = node->level;
    }

    // If we have a vars block we need to add it to our stack
    if (node->token.block == "<vars>") {
        blockStack[blockCount].varsInBlock++;

        // We need to check if the number is negative
        int numToBe;
        if (variableIdentifiers[3] == "-") {
            numToBe = stoi(variableIdentifiers[4]) * -1;
        } else {
            numToBe = stoi(variableIdentifiers[3]);
        }

        // Insert the variable into our array
        Parser::insertVar(variableIdentifiers[1], node->token.lineNumber, node->token.lineNumber, numToBe);

    // If we have anything but a vars block we need to search to see if it was declared
    } else if (node->token.block == "<R>") {
        Parser::searchVar(variableIdentifiers[0], node->token.lineNumber);
    } else if (node->token.block == "<label>" || node->token.block == "<in>" || node->token.block == "<goto>" ||
               node->token.block == "<assign>" || node->token.block == "<label>") {
        Parser::searchVar(variableIdentifiers[1], node->token.lineNumber);
    }
}

/**
 * Since our user input is all one string (mistakenly made this way and it was too hard to change it) we need to
 * separate it into individual strings
 * @param node
 * @return
 */
string* Parser::splitUserInput(struct Node* node) {
    static string variableIdentifiers[99];
    string word;
    int count = 0;

    // Loop through the input
    for (auto x : node->token.userInput) {
        // Check for spaces or semicolons between words
        if (x != ' ' && x != ';') {
            word += x;

        // Otherwise we concatenate the letter
        } else {
            variableIdentifiers[count] = word;

            // If we find began we cannot have global variables anymore
            if (word == "begin") {
                isGlobal = false;
            }

            word = "";
            count++;
        }
    }

    // There is no word so reset
    if (!word.empty()) {
        variableIdentifiers[count] = word;

        if (word == "begin") {
            isGlobal = false;
        }
    }

    return variableIdentifiers;
}

/**
 * Here we get a temporary storage variable by concatenating i++.
 * @return
 */
string Parser::newTemp() {
    std::ostringstream tempVariable;
    tempVariable << "T" << varCt++;

    vars[varTotal].value = 0;
    vars[varTotal].name = tempVariable.str();
    varTotal++;

    return tempVariable.str();
}

/**
 * Here we get a temporary storage label by concatenating i++.
 * Used to branch inside our ASM.
 * @return
 */
string Parser::newTempLabel() {
    std::ostringstream tempLabel;
    tempLabel << "L" << labelCt++;

    return tempLabel.str();
}

/**
 * We insert the variable onto the stack to keep track of it throughout the traversal.
 * @param identifier
 * @param identifierLine
 * @param lineNumber
 */
void Parser::insertVar(string identifier, int identifierLine, int lineNumber, int value) {
    // Check to make sure it is not already in stack
    for (int i = 0; i < varTotal; i++) {
        if (varStack[i].name == identifier) {
            // We get 2 situations here. Either the variable is global and there is another global or
            // it isn't global and is in the same block
            if ((isGlobal && varStack[i].isGlobal) ||
                (!isGlobal && !varStack[i].isGlobal && varStack[i].blockCount == blockCount)) {
                cout << "[ERROR]: Variable " << identifier << " on line " << identifierLine <<
                        " was previously declared on line " << varStack[i].lineNumber << endl;

                exit(1);
            }
        }
    }
    // Also insert into variable array
    vars[varTotal].value = value;
    vars[varTotal].name = identifier;

    // Insert the name, if it is global, and the line number into the stack
    varStack[varTotal].name = std::move(identifier);
    varStack[varTotal].lineNumber = lineNumber;
    varStack[varTotal].isGlobal = isGlobal;
    varStack[varTotal].blockCount = blockCount;

    // Increase our total
    varTotal++;
}

/**
 * Here we remove the variables from the top of the stack. We do this because the block has ended.
 */
void Parser::popVar() {
    for (int i = blockStack[blockCount].varsInBlock; i > 0; i--) {
        // Decrease our total
        varTotal--;
    }

    // Reset the struct
    blockStack[blockCount].varsInBlock = 0;
    blockCount--;
}

/**
 * Here we search for our variable to ensure that it has been declared
 * @param identifier
 * @param identifierLine
 */
void Parser::searchVar(const string& identifier, int identifierLine) {
    bool isDeclared = false;

    // Identifiers must start with a letter or _
    if (identifier.empty() || isdigit(identifier[0]))
        return;

    // Check all vars in stack to see if it is there
    for (int i = 0; i < varTotal; i++) {
        if (varStack[i].name == identifier) {
            isDeclared = true;
            break;
        }
    }


    // If it is not then we error and exit
    if (!isDeclared) {
        cout << "[ERROR]: Variable " << identifier << " on line " << identifierLine << " has not been declared" << endl;
        exit(1);
    }
}

/**
 * Program token
 * <program> -> <vars> main <block>
 * @param node
 * @return
 */
struct Node *Parser::programToken(struct Node* node) {
    // If our first keyword isn't main then we have a variable
    if ((globalToken.id == "KW_tk" && globalToken.userInput != "main") || (globalToken.id == "ID_tk")) {
        node->childOne = Parser::varsToken(node->childOne, node);
        Parser::programToken(node->childOne);

    // Check for our main keyword to start
    } else if (globalToken.id == "KW_tk" && globalToken.userInput == "main") {
        globalToken = Parser::getNewToken();
        node->childTwo = Parser::blockToken(node->childTwo, node);

    // Error
    } else {
        cout << "Expected an variable or the main keyword, found " << globalToken.userInput << " on line "
        << globalToken.lineNumber << endl;

        exit(1);
    }

    return node;
}

/**
 * Block token
 * <block> -> begin <vars> <stats> end
 * @param node
 * @param originNode
 * @return
 */
struct Node *Parser::blockToken(struct Node* node, struct Node* originNode) {
    // Needs to start with begin
    if (globalToken.id == "KW_tk" && globalToken.userInput == "begin") {
        // Edit our block token
        globalToken.userInput = "begin end";
        globalToken.block = "<block>";

        // Add our block to the tree
        node = Parser::addStructure(node, globalToken, originNode);

        globalToken = Parser::getNewToken();
        if ((globalToken.id == "KW_tk" && globalToken.userInput != "main") || (globalToken.id == "ID_tk")) {
            // Have to have variable token first
            node->childOne = Parser::varsToken(node->childOne, node);

            // Do stats after variables
            node->childTwo = Parser::statsToken(node->childTwo, node);

            if (globalToken.userInput != "end") {
                cout << "Expected the end keyword, found " << globalToken.userInput << " on line "
                << globalToken.lineNumber << endl;

                exit(1);
            }
        } else {
            cout << "Expected a variable, found " << globalToken.userInput << " on line "
            << globalToken.lineNumber << endl;

            exit(1);
        }
    } else {
        cout << "Expected the begin keyword, found " << globalToken.userInput << " on line "
        << globalToken.lineNumber << endl;

        exit(1);
    }

    return node;
}

/**
 * Vars token
 * <vars> -> empty | data Identifier :=  Integer  ;  <vars>
 * @param node
 * @param originNode
 * @return
 */
struct Node *Parser::varsToken(struct Node* node, struct Node* originNode) {
    string statement;

    // Variables must start with data keyword
    if (globalToken.id == "KW_tk" && globalToken.userInput == "data") {
        statement += globalToken.userInput;
        globalToken = Parser::getNewToken();

        // Followed by an id
        if (globalToken.id == "ID_tk") {
            statement += " ";
            statement += globalToken.userInput;

            globalToken = Parser::getNewToken();

            // Followed by an :=
            if (globalToken.id == "OP_tk" && globalToken.userInput == ":=") {
                statement += " ";
                statement += globalToken.userInput;

                globalToken = Parser::getNewToken();

                // The number could be negative
                if (globalToken.userInput == "-") {
                    statement += " ";
                    statement += globalToken.userInput;

                    globalToken = Parser::getNewToken();
                }

                // Followed by a number
                if (globalToken.id == "NUM_tk") {
                    statement += " ";
                    statement += globalToken.userInput;

                    globalToken = Parser::getNewToken();

                    // Ended with a semicolon
                    if (globalToken.id == "DEL_tk" && globalToken.userInput == ";") {
                        statement += globalToken.userInput;

                        globalToken.userInput = statement;
                        globalToken.block = "<vars>";

                        node = Parser::addStructure(node, globalToken, originNode);
                    } else {
                        cout << "Expected a semicolon, found " << globalToken.userInput << " on line "
                        << globalToken.lineNumber << endl;

                        exit(1);
                    }
                } else {
                    cout << "Expected an integer, found " << globalToken.userInput << " on line "
                    << globalToken.lineNumber << endl;

                    exit(1);
                }
            } else {
                cout << "Expected a := delimiter, found " << globalToken.userInput << " on line "
                << globalToken.lineNumber << endl;

                exit(1);
            }
        } else {
            cout << "Expected an identifier, found " << globalToken.userInput << " on line "
            << globalToken.lineNumber << endl;

            exit(1);
        }
    } else {
        return node;
    }

    // Since we can have multiple variables in a row we should check
    // for another directly after
    statement = "";
    globalToken = Parser::getNewToken();
    if (globalToken.id == "KW_tk" && globalToken.userInput == "data") {
        node->childOne = Parser::varsToken(node->childOne, node);
    }


    // We are done with variables so return
    return node;
}

/**
 * Expr token
 * <expr> -> <N> - <expr>  | <N>
 * @param node
 * @param originNode
 * @return
 */
struct Node *Parser::exprToken(struct Node* node, struct Node* originNode) {
    // We need to add a node a head of time so we can pass down the children
    Token fakeToken;
    fakeToken.block = "<expr>";
    fakeToken.lineNumber = globalToken.lineNumber;

    node = Parser::addStructure(node, fakeToken, originNode);

    // Our first token has to be a n token
    node->childOne = Parser::nToken(node->childOne, node);

    // After we return we can have a - token
    if (globalToken.id == "OP_tk" && globalToken.userInput == "-") {
        // We need to edit our root node
        node = Parser::addStructure(node, fakeToken, originNode, globalToken.userInput);

        globalToken = getNewToken();
        node->childTwo = Parser::exprToken(node->childTwo, node);
    }

    return node;
}

/**
 * N token
 * <N> -> <A> / <N> | <A> * <N> | <A>
 * @param node
 * @param originNode
 * @return
 */
struct Node *Parser::nToken(struct Node* node, struct Node* originNode) {
    // We need to add a node a head of time so we can pass down the children
    Token fakeToken;
    fakeToken.block = "<N>";
    fakeToken.lineNumber = globalToken.lineNumber;
    node = Parser::addStructure(node, fakeToken, originNode);

    node->childOne = Parser::aToken(node->childOne, node);

    if (globalToken.id == "OP_tk" && (globalToken.userInput == "/" || globalToken.userInput == "*")) {
        node = Parser::addStructure(node, fakeToken, originNode, globalToken.userInput);

        globalToken = getNewToken();
        node->childTwo = Parser::nToken(node->childTwo, node);
    }

    return node;
}

/**
 * A token
 * <A> -> <M> + <A> | <M>
 * @param node
 * @param originNode
 * @return
 */
struct Node *Parser::aToken(struct Node* node, struct Node* originNode) {
    Token fakeToken;
    fakeToken.block = "<A>";
    fakeToken.lineNumber = globalToken.lineNumber;
    node = Parser::addStructure(node, fakeToken, originNode);

    node->childOne = Parser::mToken(node->childOne, node);

    globalToken = getNewToken();
    if (globalToken.id == "OP_tk" && globalToken.userInput == "+") {
        node = Parser::addStructure(node, fakeToken, originNode, globalToken.userInput);

        globalToken = getNewToken();
        node->childTwo = Parser::aToken(node->childTwo, node);
    }

    return node;
}

/**
 * M token
 * <M> -> * <M> |  <R>
 * @param node
 * @param originNode
 * @return
 */
struct Node *Parser::mToken(struct Node* node, struct Node* originNode) {
    Token fakeToken;
    fakeToken.block = "<M>";
    fakeToken.lineNumber = globalToken.lineNumber;

    // Here we can have a * and if we don;t we just go to the r token
    if (globalToken.id == "OP_tk" && globalToken.userInput == "*") {
        fakeToken.userInput = "*";
        node = Parser::addStructure(node, fakeToken, originNode);

        globalToken = getNewToken();
        node->childOne = Parser::mToken(node->childOne, node);

    // Didn't have a * so we are going to r token
    } else {
        node = Parser::addStructure(node, fakeToken, originNode);
        node->childOne = Parser::rToken(node->childOne, node);
    }

    return node;
}

/**
 * R token
 * <R> -> ( <expr> ) | Identifier | Integer
 * @param node
 * @param originNode
 * @return
 */
struct Node *Parser::rToken(struct Node* node, struct Node* originNode) {
    Token fakeToken;
    fakeToken.block = "<R>";
    fakeToken.lineNumber = globalToken.lineNumber;

    // If we find a ( it means we are going into an expression
    if (globalToken.id == "DEL_tk" && globalToken.userInput == "(") {
        fakeToken.userInput = "( )";
        node = Parser::addStructure(node, fakeToken, originNode);

        globalToken = getNewToken();
        node->childOne = Parser::exprToken(node->childOne, node);

        // Must have closing )
        if (globalToken.userInput != ")") {
            cout << "Expected a closing parenthesis, found " << globalToken.userInput << " on line "
            << globalToken.lineNumber << endl;

            exit(1);
        }

    // We can also have a number or id
    } else if (globalToken.id == "ID_tk" || globalToken.id == "NUM_tk") {
        fakeToken.userInput = globalToken.userInput;
        node = Parser::addStructure(node, fakeToken, originNode);

    // Found nothing of use so error
    } else {
        cout << "Expected an ( expression ), identifier, or integer found " << globalToken.userInput << " on line "
        << globalToken.lineNumber << endl;

        exit(1);
    }

    return node;
}

/**
 * Stats token
 * <stats> -> <stat> <mStat>
 * @param node
 * @param originNode
 * @return
 */
struct Node *Parser::statsToken(struct Node* node, struct Node* originNode) {
    Token fakeToken;
    fakeToken.block = "<stats>";
    fakeToken.lineNumber = globalToken.lineNumber;

    node = Parser::addStructure(node, fakeToken, originNode);

    node->childOne = Parser::statToken(node->childOne, node);

    globalToken = Parser::getNewToken();
    node->childTwo = Parser::mStatToken(node->childTwo, node);

    return node;
}

/**
 * mStat token
 * <mStat> -> empty | <stat> <mStat>
 * @param node
 * @param originNode
 * @return
 */
struct Node *Parser::mStatToken(struct Node* node, struct Node* originNode) {
    // if we have the end keyword we know mstat is empty
    if (globalToken.id == "KW_tk" && globalToken.userInput != "end") {
        Token fakeToken;
        fakeToken.block = "<mStat>";
        fakeToken.lineNumber = globalToken.lineNumber;

        node = Parser::addStructure(node, fakeToken, originNode);

        // We need to check for all of our keywords that lead us to a new token
        if (globalToken.userInput == "getter" || globalToken.userInput == "outter" || globalToken.userInput == "if" ||
            globalToken.userInput == "loop" || globalToken.userInput == "assign" || globalToken.userInput == "void" ||
            globalToken.userInput == "proc" || globalToken.userInput == "begin") {
            node->childOne = Parser::statToken(node->childOne, node);
        }

        // We can always loop back into mstat
        globalToken = Parser::getNewToken();
        node->childTwo = Parser::mStatToken(node->childTwo, node);
    }

    return node;
}

/**
 * Stat token
 * <stat> -> <in> ;  | <out> ;  | <block>| <if> ;  | <loop> ;  | <assign> ; | <goto> ; | <label> ;
 * @param node
 * @param originNode
 * @return
 */
struct Node *Parser::statToken(struct Node* node, struct Node* originNode) {
    Token fakeToken;
    fakeToken.block = "<stat>";
    fakeToken.userInput = ";";
    fakeToken.lineNumber = globalToken.lineNumber;

    node = Parser::addStructure(node, fakeToken, originNode);

    // Check if our token is one of the stat keywords so we can move on
    if (globalToken.id == "KW_tk" && globalToken.userInput == "getter") {
        node->childOne = Parser::inToken(node->childOne, node);
        globalToken = getNewToken();
    } else if (globalToken.id == "KW_tk" && globalToken.userInput == "outter") {
        node->childOne = Parser::outToken(node->childOne, node);
    } else if (globalToken.id == "KW_tk" && globalToken.userInput == "if") {
        node->childOne = Parser::ifToken(node->childOne, node);
    } else if (globalToken.id == "KW_tk" && globalToken.userInput == "loop") {
        node->childOne = Parser::loopToken(node->childOne, node);
    } else if (globalToken.id == "KW_tk" && globalToken.userInput == "assign") {
        node->childOne = Parser::assignToken(node->childOne, node);
    } else if (globalToken.id == "KW_tk" && globalToken.userInput == "void") {
        node->childOne = Parser::labelToken(node->childOne, node);
        globalToken = getNewToken();
    } else if (globalToken.id == "KW_tk" && globalToken.userInput == "proc") {
        node->childOne = Parser::gotoToken(node->childOne, node);
        globalToken = getNewToken();
    } else if (globalToken.id == "KW_tk" && globalToken.userInput == "begin") {
        node->childOne = Parser::blockToken(node->childOne, node);
    } else {
        cout << "Expected a keyword (getter, outter, if, etc), found " << globalToken.userInput << " on line "
        << globalToken.lineNumber << endl;

        exit(1);
    }

    return node;
}

/**
 * In Token
 * <in> -> getter  Identifier
 * @param node
 * @param originNode
 * @return
 */
struct Node *Parser::inToken(struct Node* node, struct Node* originNode) {
    string statement = "getter ";

    globalToken = getNewToken();
    globalToken.block = "<in>";

    // We have to have an identifier on this one
    if (globalToken.id == "ID_tk") {
        statement += globalToken.userInput;
        globalToken.userInput = statement;

        node = Parser::addStructure(node, globalToken, originNode);
    } else {
        cout << "Expected a identifier, found " << globalToken.userInput << " on line " << globalToken.lineNumber
        << endl;

        exit(1);
    }

    return node;
}

/**
 * Out Token
 * <out> -> outter <expr>
 * @param node
 * @param originNode
 * @return
 */
struct Node *Parser::outToken(struct Node* node, struct Node* originNode) {
    Token fakeToken;
    fakeToken.block = "<out>";
    fakeToken.userInput = "outter";
    fakeToken.lineNumber = globalToken.lineNumber;

    node = Parser::addStructure(node, fakeToken, originNode);

    globalToken = getNewToken();
    node->childOne = Parser::exprToken(node->childOne, node);

    return node;
}

/**
 * If Token
 * <if> -> if [ <expr> <RO> <expr> ]then <stat>
 * @param node
 * @param originNode
 * @return
 */
struct Node *Parser::ifToken(struct Node* node, struct Node* originNode) {
    Token fakeToken;
    fakeToken.block = "<if>";
    fakeToken.userInput = "if [ ] then";
    fakeToken.lineNumber = globalToken.lineNumber;

    node = Parser::addStructure(node, fakeToken, originNode);

    globalToken = getNewToken();

    // Need to check for bracket
    if (globalToken.id == "DEL_tk" && globalToken.userInput == "[") {
        globalToken = getNewToken();

        // Now we do the three tokens inside the brackets
        node->childOne = Parser::exprToken(node->childOne, node);

        node->childTwo = Parser::r0Token(node->childTwo, node);

        globalToken = getNewToken();
        node->childThree = Parser::exprToken(node->childThree, node);

        // We have to have a closing bracket
        if (globalToken.id == "DEL_tk" && globalToken.userInput == "]") {
            globalToken = getNewToken();
            if (globalToken.id == "KW_tk" && globalToken.userInput == "then") {
                globalToken = getNewToken();
                node->childFour = Parser::statToken(node->childFour, node);

                globalToken = getNewToken();

                // Also end with a semicolon
                if (globalToken.id != "DEL_tk" && globalToken.userInput != ";") {
                    cout << "Expected a semicolon, found " << globalToken.userInput << " on line "
                    << globalToken.lineNumber << endl;

                    exit(1);
                }
            } else {
                cout << "Expected the then keyword, found " << globalToken.userInput << " on line "
                << globalToken.lineNumber << endl;

                exit(1);
            }
        } else {
            cout << "Expected a closing bracket, found " << globalToken.userInput << " on line "
            << globalToken.lineNumber << endl;

            exit(1);
        }
    } else {
        cout << "Expected an open bracket, found " << globalToken.userInput << " on line " << globalToken.lineNumber
        << endl;

        exit(1);
    }

    return node;
}

/**
 * Loop Token
 * <loop> -> loop  [ <expr> <RO> <expr> ]  <stat>
 * @param node
 * @param originNode
 * @return
 */
struct Node *Parser::loopToken(struct Node* node, struct Node* originNode) {
    Token fakeToken;
    fakeToken.block = "<loop>";
    fakeToken.userInput = "loop [ ]";
    fakeToken.lineNumber = globalToken.lineNumber;

    node = Parser::addStructure(node, fakeToken, originNode);

    globalToken = getNewToken();

    // This is similar to the if token
    if (globalToken.id == "DEL_tk" && globalToken.userInput == "[") {
        globalToken = getNewToken();
        node->childOne = Parser::exprToken(node->childOne, node);

        node->childTwo = Parser::r0Token(node->childTwo, node);

        globalToken = getNewToken();
        node->childThree = Parser::exprToken(node->childThree, node);

        if (globalToken.id == "DEL_tk" && globalToken.userInput == "]") {
            globalToken = getNewToken();
            node->childFour = Parser::statToken(node->childFour, node);

            globalToken = getNewToken();
            if (globalToken.id != "DEL_tk" || globalToken.userInput != ";") {
                cout << "Expected a semicolon, found " << globalToken.userInput << " on line "
                     << globalToken.lineNumber << endl;

                exit(1);
            }
        } else {
            cout << "Expected a closing bracket, found " << globalToken.userInput << " on line "
            << globalToken.lineNumber << endl;

            exit(1);
        }
    } else {
        cout << "Expected an open bracket, found " << globalToken.userInput << " on line " << globalToken.lineNumber
        << endl;

        exit(1);
    }

    return node;
}

/**
 * Assign Token
 * <assign> -> assignIdentifier  := <expr>
 * @param node
 * @param originNode
 * @return
 */
struct Node *Parser::assignToken(struct Node* node, struct Node* originNode) {
    string statement;

    // Same as var token but substituting data for assign
    if (globalToken.id == "KW_tk" && globalToken.userInput == "assign") {
        statement += globalToken.userInput;
        statement += " ";

        globalToken = getNewToken();
        if (globalToken.id == "ID_tk") {
            statement += globalToken.userInput;
            statement += " ";

            globalToken = getNewToken();
            if (globalToken.id == "OP_tk" && globalToken.userInput == ":=") {
                statement += globalToken.userInput;
                globalToken.userInput = statement;
                globalToken.block = "<assign>";

                node = Parser::addStructure(node, globalToken, originNode);

                globalToken = getNewToken();
                node->childOne = Parser::exprToken(node->childOne, node);
            } else {
                cout << "Expected an := delimiter, found " << globalToken.userInput << " on line "
                << globalToken.lineNumber << endl;

                exit(1);
            }
        } else {
            cout << "Expected an identifier, found " << globalToken.userInput << " on line "
            << globalToken.lineNumber << endl;

            exit(1);
        }
    } else {
        cout << "Expected the keyword assign, found " << globalToken.userInput << " on line "
        << globalToken.lineNumber << endl;

        exit(1);
    }

    return node;
}

/**
 * R0 Token
 * <RO> -> =>  | =<|  ==  |   [ == ]  (three tokens)  | %
 * @param node
 * @param originNode
 * @return
 */
struct Node *Parser::r0Token(struct Node* node, struct Node* originNode) {
    // Here we just look for our operator tokens
    if (globalToken.id == "OP_tk" && (globalToken.userInput == "=>" || globalToken.userInput == "=<"
        || globalToken.userInput == "==" || globalToken.userInput == "%")) {
        globalToken.block = "<R0>";
        node = Parser::addStructure(node, globalToken, originNode);

    // The [==] token requires 3 different tokens combined so we need to check for all 3
    } else if (globalToken.id == "DEL_tk" && globalToken.userInput == "[") {
        string statement = globalToken.userInput;
        statement += " ";
        globalToken = Parser::getNewToken();
        if (globalToken.userInput == "==") {
            statement += globalToken.userInput;
            globalToken = Parser::getNewToken();
            if (globalToken.userInput == "]") {
                statement += " ";
                statement += globalToken.userInput;
                globalToken.block = "<R0>";
                globalToken.userInput = statement;
                node = Parser::addStructure(node, globalToken, originNode);
            } else {
                cout << "Expected a closing bracket, found " << globalToken.userInput << " on line "
                << globalToken.lineNumber << endl;

                exit(1);
            }
        } else {
            cout << "Expected ==, found " << globalToken.userInput << " on line " << globalToken.lineNumber << endl;
            exit(1);
        }
    } else {
        cout << "Expected an operator or an open bracket, found " << globalToken.userInput << " on line "
        << globalToken.lineNumber << endl;

        exit(1);
    }

    return node;
}

/**
 * Label Token
 * <label> -> void Identifier
 * @param node
 * @param originNode
 * @return
 */
struct Node *Parser::labelToken(struct Node* node, struct Node* originNode) {
    string statement = "void ";

    globalToken = getNewToken();
    globalToken.block = "<label>";

    // Only need an identifier since we found void in the stat token
    if (globalToken.id == "ID_tk") {
        statement += globalToken.userInput;
        globalToken.userInput = statement;

        node = Parser::addStructure(node, globalToken, originNode);
    } else {
        cout << "Expected a identifier, found " << globalToken.userInput << " on line " << globalToken.lineNumber
        << endl;

        exit(1);
    }

    return node;
}

/**
 * Goto Token
 * <goto> -> proc Identifier
 * @param node
 * @param originNode
 * @return
 */
struct Node *Parser::gotoToken(struct Node* node, struct Node* originNode) {
    string statement = "proc ";

    globalToken = getNewToken();
    globalToken.block = "<goto>";

    // Only need an identifier since we found proc in stat token
    if (globalToken.id == "ID_tk") {
        statement += globalToken.userInput;
        globalToken.userInput = statement;

        node = Parser::addStructure(node, globalToken, originNode);
    } else {
        cout << "Expected a identifier, found " << globalToken.userInput << " on line " << globalToken.lineNumber
        << endl;

        exit(1);
    }

    return node;
}

/**
 * Here we can grab our next token from our scanner
 * @return
 */
Token Parser::getNewToken() {
    Token returnToken;

    do {
        // Grab our character and look ahead
        character = getc(file);
        lookAhead = getc(file);

        // Call the scanner
        returnToken = Scanner::scan(file, character, lookAhead);
    } while (returnToken.successId == 0);

    // Return to the parser
    return returnToken;
}