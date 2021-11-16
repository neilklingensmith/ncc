


#include "parser.h"



parser::parser(char *fname) {
    this->lex = new lexicalScanner(fname);
    this->file_name = fname;
    lex->getNextLexeme(); // Read the first lexeme from the file
}

void parser::error(const char *msg) {
    std::cerr << msg << "\n";
    exit(1);
}

/*
 * block
 *
 * Handles a block of statements enclosed by `{' and `}'
 *
 *
 *
 */
void parser::block() {
    
    lexeme l = lex->peekLexeme();

    // Expected: open brace
    if(l.getType() != LEXEME_TYPE_OPENBRACE) {
        std::cerr << "[block] got " << l.getType() << std::endl;
        this->error("Expected: `{'.");
    }
    // Eat the open brace
    l = lex->getNextLexeme();

    // Keep eating statements from the block until we encounter a close brace `}'
    while(this->lex->peekLexeme().getType() != LEXEME_TYPE_CLOSEBRACE) {
        std::cerr << "Next lexeme type is " << this->lex->peekLexeme().getType() << "\n";
        this->statement(); // Process statement
    }
    // Expected: close brace
    l = lex->getNextLexeme();
    if(l.getType() != LEXEME_TYPE_CLOSEBRACE) {
        std::cerr << "[block] got " << l.getType() << std::endl;
        this->error("Expected: `}'.");
    }


}

void parser::statement() {
    std::cout << "\n\nProcessing statement..." << std::endl;

    
    lexeme l ;

    // Eat up all the lexemes in the statement...
    while(this->lex->peekLexeme().getType() != LEXEME_TYPE_SEMICOLON) {
        std::cout << "[statement] peekLexeme.getType() = " << this->lex->peekLexeme().getType() << std::endl;
        l = lex->getNextLexeme();
    }
    // Eat the semicolong lexeme...
    if(this->lex->peekLexeme().getType() == LEXEME_TYPE_SEMICOLON) {
        l = lex->getNextLexeme();
    }
    std::cout << "Done processing statement...\n\n";
}
