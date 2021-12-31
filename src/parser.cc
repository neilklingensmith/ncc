


#include "parser.h"
#include <map>

parser::parser(char *ifname, char *ofname) {
    input_file_name = new std::string(ifname);
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
    std::cerr << *input_file_name << " Line " << lex->getCurrLineNumber() << " Col " << lex->getCurrColumnNumber() << ": " << msg << "\n";
    exit(1);
}

void parser::function() {
    char buf[200];

    lexeme returntype =  lex->getNextLexeme(); // Get the function's return type
    lexeme funcname = lex->getNextLexeme(); // Get function name

    std::cerr << "Got function named " << funcname.getText() << "\n";
    snprintf(buf, sizeof(buf), "%s:", funcname.getText().c_str());
    emit(buf);

    lexeme parens = lex->getNextLexeme(); // Get open paren
    parens = lex->getNextLexeme(); // Get close paren
    block();
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
    // Iterate thru the declarations. Count the total size of the stack frame to be created, and mark the location of each identifier on the stack.
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
    emit((char*)"MOVEM.L D1-D7/A0-A5,-(A7)");
    // Done iterating thru declarations
    /////////////////////////////////////////

    // Keep eating statements from the block until we encounter a close brace `}'
    while(this->lex->peekLexeme().getType() != LEXEME_TYPE_CLOSEBRACE) {
//        std::cerr << "Next lexeme type is " << this->lex->peekLexeme().getType() << "\n";
        this->statement(symbolTable); // Process statement
    }

    emit((char*)"MOVEM.L (A7)+,D1-D7/A0-A5");
    emit((char*)"UNLK A6");
    emit((char*)"RTS");
    // Expected: close brace
    l = lex->getNextLexeme();
    if(l.getType() != LEXEME_TYPE_CLOSEBRACE) {
        std::cerr << "[block] got " << l.getType() << std::endl;
        this->error("Expected: `}'.");
    }

}


void parser::emit(char *s) {
     *os << s << std::endl;
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
//        std::cerr << "[parser::declaration] Got semicolon \"" << this->lex->peekLexeme().getText() << "\" type = "<< this->lex->peekLexeme().getType() << std::endl;
        l = lex->getNextLexeme();
    } else {
//        std::cerr << "[parser::declaration] DIDN'T GET SEMICOLON!! GOT " << this->lex->peekLexeme().getType() << " INSTEAD!!" << std::endl;
        char msg[200];
        snprintf(msg, 200, "Expected semicolon at end of declaration. Got `%s' instead.", this->lex->peekLexeme().getText().c_str());
        error(msg);
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
//    std::cout << "\n\nProcessing statement..." << std::endl;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create a stack of data registers and address registers that are available for use in this statement
    std::string dataRegNames[] = { "D7", "D6", "D5", "D4", "D3", "D2", "D1", "D0" };
    std::string addrRegNames[] = { "A5", "A4", "A3", "A2", "A1", "A0" }; // A6 and A7 are the frame pointer and stack pointer, so can't use them.
    std::stack<std::string>dataRegFreeStack; // Data registers that are available for use in this statement
    std::stack<std::string>dataRegStatementStack; // 
    std::stack<std::string>addrRegFreeStack;
    std::stack<std::string>addrRegStatementStack;
    for (auto& item : dataRegNames) {
        dataRegFreeStack.push(item);
    }
    for (auto& item : addrRegNames) {
        addrRegFreeStack.push(item);
    }



    lexeme l ;

    // This if block deals with statements of the form:
    //
    //      ident = expr;
    //
    // The first bit processes the identifier by finding it in the symbol table.
    if(this->lex->peekLexeme().getType() == LEXEME_TYPE_IDENT) {
//        std::cerr << "[parser::statement] found identifier \"" << this->lex->peekLexeme().getText() << "\"" << std::endl;
        // Look up the identifier in the symbol table for this block.
        identifier *id = symbolTable[this->lex->peekLexeme().getText()];
        if(id == NULL) {
            // Check to make sure the symbol is in our symbol table.
            char msg[100];
            snprintf(msg, sizeof(msg), "Identifier `%s' undeclared.", this->lex->peekLexeme().getText().c_str());
            this->error(msg);
        }
//        std::cerr << "[parser::statement] @fp" << id->getStackFramePosition() << std::endl;
        lexeme idlexeme = this->lex->getNextLexeme(); // Eat the identifier
        l = this->lex->getNextLexeme(); // Eat the `='
        if(l.getType() != LEXEME_TYPE_ASSIGNMENTOP) {
            std::cerr << "[parser::statement] Got lexeme type " << l.getType() << std::endl;
            this->error("Expected `=' in assignment");
        }
        // Process the expression
        this->expression(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack);

        // Store the result on the stack at the location allocated to the identifier
        char msg[200];
        snprintf(msg, sizeof(msg), "MOVE %s,%d(A6)", dataRegStatementStack.top().c_str(), id->getStackFramePosition());
        emit(msg);
    } else if ((this->lex->peekLexeme().getType() == LEXEME_TYPE_KEYWORD) && (this->lex->peekLexeme().getSubtype() == KEYWORD_TYPE_RETURN)) {
        // Handle return statements
        char msg[200];
        lexeme keywordlexeme = this->lex->getNextLexeme(); // Eat the return keyword
        lexeme returnlexeme = this->lex->getNextLexeme(); // Eat the thing to return
        if(returnlexeme.getType() == LEXEME_TYPE_INTEGER) { // constant return value
            snprintf(msg, sizeof(msg), "MOVE %d,D0", returnlexeme.getValue());
        } else if (returnlexeme.getType() == LEXEME_TYPE_IDENT) { // identifier's value returned
            identifier *id = symbolTable[returnlexeme.getText()];
            snprintf(msg, sizeof(msg), "MOVE %d(A6),D0", id->getStackFramePosition());
        }
        emit(msg);
    } else {
        this->error("Expected: identifier at beginning of statement");
    }
    // Eat the semicolong lexeme...
    if(this->lex->peekLexeme().getType() == LEXEME_TYPE_SEMICOLON) {
        l = lex->getNextLexeme();
    }
//    std::cout << "Done processing statement...\n\n";
}

/*
 *
 *
 *
 *
 */
void parser::expression(std::map<std::string, identifier*>&symbolTable, std::stack<std::string>&dataRegFreeStack, std::stack<std::string>&dataRegStatementStack, std::stack<std::string>&addrRegFreeStack, std::stack<std::string>&addrRegStatementStack) {
    // expression    -> term [(`+' | `-') term]*
    // Process the first term
    this->term(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack); // Eat the identifier

    std::cerr << "[parser::expression] next lexeme type is " << this->lex->peekLexeme().getType() << "\n";

    // Check if we have an addop after the first term.
    while(this->lex->peekLexeme().getType() == LEXEME_TYPE_ADDOP) {
        std::cerr << "[parser::expression] Processing another term in expression...\n";
        lexeme sign_lexeme = this->lex->getNextLexeme(); // Get the sign (`+' or `-')
        this->term(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack); // Eat the identifier

        // Now add or subtract the terms
        char msg[200];
        std::string op1, op2;
        op1 = dataRegStatementStack.top();
        dataRegStatementStack.pop();
        op2 = dataRegStatementStack.top();
        dataRegStatementStack.pop();

        if(sign_lexeme.getText() == "+") {
            snprintf(msg,sizeof(msg), "ADD %s,%s", op2.c_str(), op1.c_str());
        } else if (sign_lexeme.getText() == "-") {
            snprintf(msg,sizeof(msg), "SUB %s,%s", op2.c_str(), op1.c_str());
        } else {
            snprintf(msg, sizeof(msg), "Error: expected `+' or `-' in expression. Got \"%s\"", sign_lexeme.getText().c_str());
            error(msg);
        }
        emit(msg);
        dataRegFreeStack.push(op1); // Reclaim one of the data registers
        dataRegStatementStack.push(op2);

    }
/*
    while(this->lex->peekLexeme().getType() != LEXEME_TYPE_SEMICOLON) {
        lexeme l;
        l = lex->getNextLexeme();
    }
*/
}

/*
 *
 *
 *
 *
 */
void parser::term(std::map<std::string, identifier*>&symbolTable, std::stack<std::string>&dataRegFreeStack, std::stack<std::string>&dataRegStatementStack, std::stack<std::string>&addrRegFreeStack, std::stack<std::string>&addrRegStatementStack) {

    //term          -> signedfactor [(`*' | `/') factor]*
    this->signedfactor(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack); // Eat the identifier

    // Handle multiple factors following the first signed factor
    while(this->lex->peekLexeme().getType() == LEXEME_TYPE_MULOP) {
        std::cerr << "[parser::term] Processing another factor in term...\n";

        lexeme mulop_lexeme = this->lex->getNextLexeme(); // Get the operation (`*' or `/')
        this->factor(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack); // Process the factor

        // Now multiply or divide the terms
        char msg[200];
        std::string op1, op2;
        op1 = dataRegStatementStack.top();
        dataRegStatementStack.pop();
        op2 = dataRegStatementStack.top();
        dataRegStatementStack.pop();

        if(mulop_lexeme.getText() == "*") {
            snprintf(msg,sizeof(msg), "MULS %s,%s", op2.c_str(), op1.c_str());
        } else if (mulop_lexeme.getText() == "/") {
            snprintf(msg,sizeof(msg), "DIVS %s,%s", op2.c_str(), op1.c_str());
        } else {
            snprintf(msg, sizeof(msg), "Error: expected `*' or `/' in term. Got \"%s\"", mulop_lexeme.getText().c_str());
            error(msg);
        }
        emit(msg);
        dataRegFreeStack.push(op1); // Reclaim one of the data registers
        dataRegStatementStack.push(op2);


    }
}

void parser::signedfactor(std::map<std::string, identifier*>&symbolTable, std::stack<std::string>&dataRegFreeStack, std::stack<std::string>&dataRegStatementStack, std::stack<std::string>&addrRegFreeStack, std::stack<std::string>&addrRegStatementStack) {
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

    
    this->factor(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack);
    
    if(sign.getText() == "-") {
        // If the factor has a `-' sign in front of it, negate the value.
        std::string regname =  dataRegStatementStack.top();
        char buf[200];
        snprintf(buf, sizeof(buf), "NEG %s", regname.c_str());
        emit(buf);

    }
}


void parser::factor(std::map<std::string, identifier*>&symbolTable, std::stack<std::string>&dataRegFreeStack, std::stack<std::string>&dataRegStatementStack, std::stack<std::string>&addrRegFreeStack, std::stack<std::string>&addrRegStatementStack) {

    // factor        -> constant | identifier | `(' expression `)'

    lexeme l;
    identifier *id;
    std::string dreg; // Data register name used to process this factor
    char msg[200];

    l = this->lex->getNextLexeme(); // Get the next lexeme. Should be a constant, identifier, or open parentheses
    switch(l.getType()) {
    case LEXEME_TYPE_INTEGER:
        //std::cerr << "[parser::factor] Found integer constant. ";
        dreg = dataRegFreeStack.top();
        dataRegFreeStack.pop();
        dataRegStatementStack.push(dreg);
        //std::cerr << "allocating register " << dreg << "\n";
        snprintf(msg, sizeof(msg), "MOVE.L #%d,%s", l.getValue(), dreg.c_str());
        emit(msg);

        break;
    case LEXEME_TYPE_IDENT:
        std::cerr << "[parser::factor] Found identifier\n";

        id = symbolTable[l.getText()];
        if(id == NULL) {
            // Check to make sure the symbol is in our symbol table.
            snprintf(msg, sizeof(msg), "Identifier `%s' undeclared.", this->lex->peekLexeme().getText().c_str());
            this->error(msg);
        }
        dreg = dataRegFreeStack.top();
        dataRegFreeStack.pop();
        dataRegStatementStack.push(dreg);
        std::cerr << "allocating register " << dreg << "\n";
        snprintf(msg, sizeof(msg), "MOVE.L %d(A6),%s", id->getStackFramePosition(), dreg.c_str());
        emit(msg);
        break;
    case LEXEME_TYPE_PARENTHESES:
        // First, ensure that we got an open paren, not a closed.
        if(l.getText() != "(") {
            snprintf(msg, sizeof(msg), "Error: Expected `(' before `%s'", this->lex->peekLexeme().getText().c_str());
            this->error(msg);
        }
        // Process the expression
        this->expression(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack);
        if(this->lex->peekLexeme().getText() != ")") {
            snprintf(msg, sizeof(msg), "Error: Expected `)' at the end of the expression. Got `%s' instead.", this->lex->peekLexeme().getText().c_str());
            this->error(msg);
        } else {
            l = this->lex->getNextLexeme(); // Eat the close paren
        }
        break;
    default:
        snprintf(msg, sizeof(msg), "Error processing factor. Expected integer constant or identifier. Got `%s'", l.getText().c_str());
        error(msg);
        break;
    }
    // Eat up all the lexemes in the statement...
#if 0
    while(this->lex->peekLexeme().getType() != LEXEME_TYPE_SEMICOLON) {
//        std::cout << "[signedfactor] peekLexeme.getType() = " << this->lex->peekLexeme().getType() << std::endl;
        l = lex->getNextLexeme();
    }
#endif

}
