


#include "parser.h"
#include <map>


parser::parser(char *ifname, char *ofname) {
    this->lex = new lexicalScanner(ifname);
    this->file_name = ifname;
    lex->getNextLexeme(); // Read the first lexeme from the file
    if(ofname != NULL) {
        os = new std::ofstream(ofname);
    } else {
        os = &std::cout;
    }
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
    int totalBytesInStackFrame = 0;
    std::map<std::string,identifier*> symbolTable;

    lexeme l = lex->peekLexeme();

    // Expected: open brace
    if(l.getType() != LEXEME_TYPE_OPENBRACE) {
        std::cerr << "[block] got " << l.getType() << std::endl;
        this->error("Expected: `{'.");
    }
    // Eat the open brace
    l = lex->getNextLexeme();

    // Process declarations
    while((this->lex->peekLexeme().getType() == LEXEME_TYPE_KEYWORD) && (this->lex->peekLexeme().getSubtype() == KEYWORD_TYPE_INT)) {
        // Found a declaration
        this->declaration(symbolTable);
    }

    //////////////////////////////////////////
    // Iterate thru the declarations
    std::map<std::string, identifier*>::iterator it;

    for (auto const& [key, val] : symbolTable) {
        totalBytesInStackFrame += val->getNumBytes();
        val->setStackFramePosition(-totalBytesInStackFrame);
        std::cerr << "identifier name \"" << key    // string (key)
              << "\" : "
              << " Num bytes " << val->getNumBytes()   // string's value
              << " Stack Frame loc " << val->getStackFramePosition()
              << std::endl;
    }

    std::string linkinstr("LINK A6,#");
    linkinstr.append(std::to_string(-totalBytesInStackFrame));
    emit(linkinstr);
    // Done iterating thru declarations
    /////////////////////////////////////////

    // Keep eating statements from the block until we encounter a close brace `}'
    while(this->lex->peekLexeme().getType() != LEXEME_TYPE_CLOSEBRACE) {
        std::cerr << "Next lexeme type is " << this->lex->peekLexeme().getType() << "\n";
        this->statement(symbolTable); // Process statement
    }
    // Expected: close brace
    l = lex->getNextLexeme();
    if(l.getType() != LEXEME_TYPE_CLOSEBRACE) {
        std::cerr << "[block] got " << l.getType() << std::endl;
        this->error("Expected: `}'.");
    }


}


void parser::emit(std::string &s) {
     *os << s << std::endl;
}

/*
 * declaration
 *
 * Processes declarations and adds identifiers to the symbol table.
 *
* Inputs:
 *
 *   symbolTable is the block's symbol table.
 */
void parser::declaration(std::map<std::string, identifier*>&symbolTable) {

    lexeme l ;
    identifier *id = new identifier;

    if(this->lex->peekLexeme().getType() == LEXEME_TYPE_KEYWORD) {
        switch(this->lex->peekLexeme().getSubtype()) {
        case KEYWORD_TYPE_INT:
            id->setType(IDENTIFIER_TYPE_INTEGER);
            id->setNumBytes(4);
            break;
        default:
            this->error("Unknown type in declaration");
        }
    } else {
        std::cerr << "[declaration] Got initial lexeme type of " << this->lex->peekLexeme().getType() << " " << this->lex->peekLexeme().getText() << std::endl;
        this->error("Expected type at beginning of the declaration.");
    }

    // Eat the datatype keyword
    lexeme datatypekeyword = this->lex->getNextLexeme();

//    std::cerr << "[declaration] Got datatype " << datatypekeyword.getText() << " number of bytes = " << id->getNumBytes() << std::endl;

    // Add the new identifier to the symbol table.
    lexeme identifiername = this->lex->getNextLexeme();
//    std::cerr << "Got identifier with name " << identifiername.getText() << std::endl;
    symbolTable.insert(std::pair<std::string,identifier*>(identifiername.getText(),id) );

    // Eat the semicolon lexeme
    if(this->lex->peekLexeme().getType() == LEXEME_TYPE_SEMICOLON) {
//        std::cerr << "[parser::declaration] Got semicolon" << std::endl;
        l = lex->getNextLexeme();
    } else {
        std::cerr << "[parser::declaration] DIDN'T GET SEMICOLON!! GOT " << this->lex->peekLexeme().getType() << " INSTEAD!!" << std::endl;
    }

}
/*
 * statement
 *
 * Processes statements, including assignments, if blocks, and while loops.
 *
 * Inputs:
 *
 *   symbolTable is the block's symbol table.
 */

void parser::statement(std::map<std::string, identifier*>&symbolTable) {
    std::cout << "\n\nProcessing statement..." << std::endl;

    
    lexeme l ;

    if(this->lex->peekLexeme().getType() == LEXEME_TYPE_IDENT) {
        std::cerr << "[parser::statement] found identifier \"" << this->lex->peekLexeme().getText() << "\"" << std::endl;
        lexeme id = this->lex->getNextLexeme(); // Eat the identifier
        l = this->lex->getNextLexeme(); // Eat the `='
        if(l.getType() != LEXEME_TYPE_ASSIGNMENTOP) {
            std::cerr << "[parser::statement] Got lexeme type " << l.getType() << std::endl;
            this->error("Expected `=' in assignment");
        }
        this->expression();
    } else {
        this->error("Expected: identifier at beginning of statement");
    }
#if 0
    else if (this->lex->peekLexeme().getType() == LEXEME_TYPE_INTEGER) {
        std::cerr << "[parser::statement] found integer " << this->lex->peekLexeme().getValue() << std::endl;
        // Eat up all the lexemes in the statement...
        while(this->lex->peekLexeme().getType() != LEXEME_TYPE_SEMICOLON) {
            std::cout << "[statement] peekLexeme.getType() = " << this->lex->peekLexeme().getType() << std::endl;
            l = lex->getNextLexeme();
        }
    } else {
        // Processing assignments
        // Eat up all the lexemes in the statement...
        while(this->lex->peekLexeme().getType() != LEXEME_TYPE_SEMICOLON) {
            std::cout << "[statement] peekLexeme.getType() = " << this->lex->peekLexeme().getType() << std::endl;
            l = lex->getNextLexeme();
        }
    }
#endif
    // Eat the semicolong lexeme...
    if(this->lex->peekLexeme().getType() == LEXEME_TYPE_SEMICOLON) {
        l = lex->getNextLexeme();
    }
    std::cout << "Done processing statement...\n\n";
}

/*
 *
 *
 *
 *
 */
void parser::expression() {
    this->signedfactor(); // Eat the identifier
}

/*
 *
 *
 *
 *
 */
void parser::term() {

}

void parser::signedfactor() {
    lexeme l, sign;
    //lexeme sf = this->lex->getNextLexeme(); // Eat the sign

    if(this->lex->peekLexeme().getType() == LEXEME_TYPE_ADDOP) {
        // Got a sign at the beginning of the factor
        sign = this->lex->getNextLexeme();
    } else {
        // If there's no sign, the implied sign is `+'
        sign.setType(LEXEME_TYPE_ADDOP);
        sign.setText("+");
    }

    // Eat up all the lexemes in the statement...
    while(this->lex->peekLexeme().getType() != LEXEME_TYPE_SEMICOLON) {
        std::cout << "[statement] peekLexeme.getType() = " << this->lex->peekLexeme().getType() << std::endl;
        l = lex->getNextLexeme();
    }


}


void parser::factor() {

}
