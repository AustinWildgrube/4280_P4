// Author: Austin Wildgrube <akwwb6@mail.umsl.edu>
// Date: 03/08/2021

#include <iostream>
#include <string>
#include <cstdio>

#include "scanner.h"

using namespace std;

// Had to move token variables into scanner cpp to avoid multiple inclusion
string keywordArray[20] = {
        "begin",
        "end",
        "loop",
        "whole",
        "void",
        "exit",
        "getter",
        "outter",
        "main",
        "if",
        "then",
        "assign",
        "data",
        "proc"
};

string tokenID[] {
        "KW_tk",
        "ID_tk",
        "NUM_tk",
        "OP_tk",
        "DEL_tk",
        "EOF_tk"
};

string tokenName[] {
        "Keyword",
        "Identifier",
        "Number",
        "Operator",
        "Delimiter",
        "End of File"
};


// We can use this array to get the correct column
char tokenArray[23] = {
        '_',
        'a',
        '1',
        '=',
        '>',
        '<',
        ':',
        '+',
        '-',
        '*',
        '/',
        '%',
        '.',
        '(',
        ')',
        ',',
        '{',
        '}',
        ';',
        '[',
        ']',
};

int stateTable[22][22] = {
        /*
            -2  : Incorrect start token
            -1  : Incorrect next token

            1000: EOF
            1001: Keyword
            1002: Identifier
            1003: Number

            1004: =
            1005: =>
            1006: =<
            1007: ==
            1008: :
            1009: :=
            1010: +
            1011: -
            1012: *
            1013: /
            1014: %
            1015: .
            1016: (
            1017: )
            1018: ,
            1019: {
            1020: }
            1021: ;
            1022: [
            1023: ]
        */
    //  { _   a     1     =     >     <     :   +   -   *   /   %   .   (   )   ,   {   }   ;   [   ]   ws   }
        { 1,  2,    3,    4,    5,    6,    7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22   }, // State 0

        { -1, 1002, 1003, -1,   -2,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -2   }, // _
        { -1, 2,    3,    -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1002 }, // a
        { -1, -1,   3,    -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1003 }, // 1
        { -1, -1,   -1,   1007, 1005, 1006, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1004 }, // =
        { -1, -1,   -1,   -1,   -2,   -2,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -2   }, // >
        { -1, -1,   -1,   -2,   -2,   -2,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -2   }, // <
        { -1, -1,   -1,   1009, -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1008 }, // :
        { -1, -1,   -1,   -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1010 }, // +
        { -1, -1,   -1,   -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1011 }, // -
        { -1, -1,   -1,   -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1012 }, // *
        { -1, -1,   -1,   -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1013 }, // /
        { -1, -1,   -1,   -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1014 }, // %
        { -1, -1,   -1,   -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1015 }, // .
        { -1, -1,   -1,   -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1016 }, // (
        { -1, -1,   -1,   -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1017 }, // )
        { -1, -1,   -1,   -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1018 }, // ,
        { -1, -1,   -1,   -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1019 }, // {
        { -1, -1,   -1,   -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1020 }, // }
        { -1, -1,   -1,   -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1021 }, // ;
        { -1, -1,   -1,   -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1022 }, // [
        { -1, -1,   -1,   -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1023 }, // ]
};

string tempString;
int token, column;
int state = 0;
int lineNumber = 1;

/**
 * This is our scanner class that determines which token a character is.
 * From here it calls a print statement within the class and will the return
 * control back to the test driver.
 * @param file
 * @param character
 * @param lookAhead
 */
Token Scanner::scan(FILE *file, char character, char lookAhead) {
    Token printStatement;

    // We need to start of checking for comments so we can immediately ignore them
    if (character == '$' && lookAhead == '$') {
        // We need to loop until we have two $$
        do {
            character = getc(file);

            if (character == '$') {
                lookAhead = getc(file);

                if (lookAhead != '$') {
                    character = getc(file);
                }
            }

            if (character == '\n') {
                lineNumber++;
            }
        } while (character != '$');

        character = getc(file);
    } else
        // Our look ahead will always go to far so we need to backtrack one step
        ungetc(lookAhead, file);

    // We need to begin checking for keywords and identifiers
    if ((isalpha(character) || character == '_')) {
        // Identifiers can only begin with a lowercase letter if it doesn't start with an underscore
        if (character != '_' && !islower(character)) {
            // If the identifier is wrongly formatted we need to collect the rest of the string so we
            // can provide it in the output
            string word;
            word.push_back(character);
            character = getc(file);

            while (!isspace(character) && character != '\n' && (isalpha(character) || isdigit(character))) {
                // Add to string
                word.push_back(character);
                character = getc(file);
            }

            Scanner::getErrorStatement(word, lineNumber);

        // We have a correctly formatter identifier
        } else {
            // If the state is not 0 and there is a space or new line just get the token without the look a head.
            if (state != 0 && (character == '\n' || isspace(character) || isalpha(character) || character == '_')) {
                token = Scanner::searchTokens(state, 21);

                ungetc(character, file);

                // Clear our temp string and reset our state
                state = 0;

                // If our token returns -2 it means that we cannot have that token by itself ( ie < )
                if (token == -2)
                    Scanner::getErrorStatement(tempString, lineNumber);
                else {
                    printStatement = Scanner::getPrintStatement(token, tempString, lineNumber);
                    tempString.clear();
                }
            } else {
                // Collect characters into a string so we know the whole identifier at the end
                string word;
                word.push_back(character);
                character = getc(file);

                while (!isspace(character) && character != '\n' && (isalpha(character) || isdigit(character))) {
                    // Add to string
                    word.push_back(character);
                    character = getc(file);
                }

                // Unget our character that broke it
                if (isspace(character) || character == '\n' || (!isalpha(character) && !isdigit(character))) {
                    ungetc(character, file);
                    state = 0;
                }

                // Check to see if the words are keywords
                if (checkKeywords(word))
                    // Is a keyword
                    return Scanner::getPrintStatement(1001, word, lineNumber);
                else
                    // Is not a keyword
                    return Scanner::getPrintStatement(1002, word, lineNumber);
            }
        }

    // The character is not a letter
    } else {
        if (character != EOF) {

            // Get columns
            if (isdigit(character))
                column = 2;
            else
                column = Scanner::getColumn(character);

            // If the getColumn function returns -99 it means that it is not a valid character
            if (column == -99 && character != '\n' && !isspace(character) && !isalpha(character)) {
                string cToS(1, character);
                Scanner::getErrorStatement(cToS, lineNumber);

            // We have a valid character so now we need to search out state table
            } else {
                // Search state table
                token = Scanner::searchTokens(state, column);

                // If the state is not 0 and there is a space or new line just get the token without the look a head.
                if (state != 0 && (character == '\n' || isspace(character) || isalpha(character))) {
                    token = Scanner::searchTokens(state, 21);

                    // If our token returns -2 it means that we cannot have that token by itself ( ie < )
                    if (token == -2)
                        Scanner::getErrorStatement(tempString, lineNumber);
                    else
                        printStatement = Scanner::getPrintStatement(token, tempString, lineNumber);

                    // Clear our temp string and reset our state
                    tempString.clear();
                    state = 0;
                } else {
                    // Token search returned another state
                    if (token > 0 && token <= 22) {
                        state = token;

                        tempString.push_back(character);

                    // We reached a final token
                    } else if (token >= 1000 && token <= 1023) {
                        if (!tempString.empty()) {
                            tempString.push_back(character);
                            printStatement = Scanner::getPrintStatement(token, tempString, lineNumber);
                            tempString.clear();
                        } else {
                            string characterToString(1, character);
                            printStatement = Scanner::getPrintStatement(token, characterToString, lineNumber);
                        }

                        state = 0;

                    // There was no white space between tokens and the two did not go together
                    } else if (token == -1) {
                        token = Scanner::searchTokens(state, 21);

                        printStatement = Scanner::getPrintStatement(token, tempString, lineNumber);
                        tempString.clear();

                        // Reset our state and put back the look ahead character
                        state = 0;
                        ungetc(character, file);
                    } else if (token == -2) {
                        if (tempString.empty()) {
                            string cToS(1, character);
                            Scanner::getErrorStatement(cToS, lineNumber);
                        } else {
                            Scanner::getErrorStatement(tempString, lineNumber);
                            state = 0;
                            ungetc(character, file);
                        }

                        tempString.clear();
                    }
                }
            }

        // We reached our end of function so we should print it
        } else {
            // Our temp string isn't empty which means we have another character to print
            if (!tempString.empty()) {
                // Check the last one
                token = Scanner::searchTokens(state, 21);

                printStatement = Scanner::getPrintStatement(token, tempString, lineNumber);
            }

            // Print our EOF
            printStatement = getPrintStatement(1000, "", lineNumber);
        }
    }

    // Increase line number if we reach a new row
    if (character == '\n') {
        lineNumber++;
    }

    return printStatement;
}

/**
 * We need to look into the token array to see if the character is a valid character in our language.
 * If it is we return the position and if it is not we return -99.
 * @param character
 * @return
 */
int Scanner::getColumn(char character) {
    // Loop through valid tokens
    for (int i = 0; i < 23; i++) {
        // Check if token is in array and return if found
        if (tokenArray[i] == character)
            return i;
    }

    // Not found
    return -99;
}

/**
 * We simply return the value found in the FSA table.
 * @param tokenState
 * @param tokenColumn
 * @return num
 */
int Scanner::searchTokens(int tokenState, int tokenColumn) {
    if (tokenColumn == -99) {
        return 0;
    }

    // return state or token
    return stateTable[tokenState][tokenColumn];
}

/**
 * We need to check our identifiers against the keywords. Here we return true if found and false if not.
 * @param word
 * @return bool
 */
bool Scanner::checkKeywords(const string &word) {
    // Loop through valid keywords
    for (auto &i : keywordArray) {
        // Check if keywords is in array and return if found
        if (i == word)
            return true;
    }

    // Not found
    return false;
}

/**
 * This is where we set the info about our token and then print it.
 * @param tokenNumber
 * @param userInput
 * @param lineNumber
 */
Token Scanner::getPrintStatement(int tokenNumber, const string& userInput, int tokenLineNumber) {
    Token returnToken;

    // Set basic token info
    returnToken.successId = tokenNumber;
    returnToken.userInput = userInput;
    returnToken.lineNumber = tokenLineNumber;
    returnToken.block = "";

    // Based on the success id we give it a specific token name/id
    if (tokenNumber == 1000) {
        returnToken.id = tokenID[5];
        returnToken.name = tokenName[5];
    } else if (tokenNumber == 1001) {
        returnToken.id = tokenID[0];
        returnToken.name = tokenName[0];
    } else if (tokenNumber == 1002) {
        returnToken.id = tokenID[1];
        returnToken.name = tokenName[1];
    } else if (tokenNumber == 1003) {
        returnToken.id = tokenID[2];
        returnToken.name = tokenName[2];
    } else if (tokenNumber >= 1004 && tokenNumber <= 1014) {
        returnToken.id = tokenID[3];
        returnToken.name = tokenName[3];
    } else if (tokenNumber >= 1015) {
        returnToken.id = tokenID[4];
        returnToken.name = tokenName[4];
    }

    // Print our token
//    cout << returnToken.lineNumber << setw(20);
//    cout << returnToken.name << setw(20);
//    cout << returnToken.id << setw(20);
//    cout << returnToken.successId << setw(20);
//    cout << returnToken.userInput << endl;
    return returnToken;
}

/**
 * Instead of handling errors in the getPrintStatement function we separated the two.
 * @param userInput
 * @param lineNumber
 */
void Scanner::getErrorStatement(const string& userInput, int tokenLineNumber) {
    cout << endl;
    cout << "SCANNER ERROR" << endl;
    cout << "Line Number " << tokenLineNumber << endl;
    cout << "Invalid Syntax " << userInput << endl;
    cout << endl;

    exit(1);
}