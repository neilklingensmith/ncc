
#include "lexicalscanner.h"
#include "lexeme.h"
#include "parser.h"
#include <iostream>
#include <fstream>


lexicalScanner *lex;
unsigned int debuglevel = 2;

void usage(char *progname) {
    std::cerr << "Usage: " << progname << " <file name>";
    exit(1);
}

int main(int argc, char **argv) {

    if (argc != 2) {
        usage(argv[0]);
        exit(1);
    }

    parser p(argv[1], NULL);

    p.function();
#if 0
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
#endif
    return 0;
}
