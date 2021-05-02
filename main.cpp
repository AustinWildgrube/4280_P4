// Author: Austin Wildgrube <akwwb6@mail.umsl.edu>
// Date: 03/08/2021

#include <iostream>
#include <fstream>
#include <string>

#include "parser.h"

using namespace std;

/**
 * This is the main function of our program where we collect command line arguments
 * and the call the function to build and print the tree.
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char** argv) {
    string inputString;
    string fileName;

    // If one argument we need to read data to a temporary text file
    if (argc == 1) {
        fileName = "temp_data.txt";
        ofstream tempFile("temp_data.txt");

        cout << "Enter a string and type quit to stop: " << endl;

        // Collect content and place in file until quit is entered
        while (getline(cin, inputString)) {
            if (inputString == "quit")
                break;
            else
                tempFile << inputString << endl;
        }
        // There is two arguments which means a file was provided
    } else if (argc == 2) {
        fileName = argv[1];

        if (fileName.substr(fileName.find_last_of('.') + 1) != "fs") {
            cout << "[Error]: File names must have extension .fs" << endl;
            return 0;
        }

    // Too many arguments were entered
    } else {
        cout << "[Error] Too many arguments were entered!" << endl;
        cout << "[Info] Usage: ./frontEnd" << endl;
        cout << "[Info] Usage: ./frontEnd [fileName].fs" << endl;
        cout << "[Info] Usage: ./frontEnd < [fileName].fs" << endl;
        return 0;
    }

    // To use ofstream it needed a string literal or a C String so we have to convert it
    auto back_to_cstr = fileName.c_str();
    Parser::parser(back_to_cstr);

    return 0;
}
