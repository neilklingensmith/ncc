
#include "lexicalscanner.h"
#include "lexeme.h"
#include <iostream>
#include <fstream>


lexicalScanner *lex;

int main(int argc, char **argv) {

    // Parse command line parameters
    if(argc == 1) {
        lex = new lexicalScanner(NULL);
    } else {
        lex = new lexicalScanner(argv[1]);
        std::cout << "File name: " << argv[1] << "\n";
    }

    while(1) {
        lex->getNextLexeme();
    }
    return 0;
}
