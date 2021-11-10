
#include "lexicalparser.h"
#include "lexeme.h"
#include <iostream>
#include <fstream>


lexicalParser *lex;

int main(int argc, char **argv) {

    // Parse command line parameters
    if(argc == 1) {
        lex = new lexicalParser(NULL);
    } else {
        lex = new lexicalParser(argv[1]);
        std::cout << "File name: " << argv[1] << "\n";
    }

    while(1) {
        lex->getNextLexeme();
    }
    return 0;
}
