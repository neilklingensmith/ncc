


#include "parser.h"
#include <map>
#include <cstring>

extern unsigned int debuglevel;


void parser::do_function_call(lexeme l, std::map<std::string, identifier*>&symbolTable, std::stack<std::string>&dataRegFreeStack, std::stack<std::string>&dataRegStatementStack, std::stack<std::string>&addrRegFreeStack, std::stack<std::string>&addrRegStatementStack) {
    // Size (in bytes) of arguments passed to the function. We will use
    // this later to collapse the stack frame after the function returns
    unsigned int argument_stack_space_size = 0;
    std::string retvaldreg, stashdreg; // Data register name used to process this factor
    char msg[200];
    // Function Call
    std::string funcname = l.getText().c_str();
    lexeme paren = this->lex->getNextLexeme(); // Eat the open paren

    // Process function arguments
    while(this->lex->peekLexeme().getType() == LEXEME_TYPE_IDENT) {
        // expression
        this->expression(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack);

        std::string argreg = dataRegStatementStack.top();
        dataRegStatementStack.pop();
        snprintf(msg, sizeof(msg), "    MOVE.L %s,-(A7)", argreg.c_str());
        argument_stack_space_size += 4; // Increment stack frame size by the size of the arg we are passing
        emit(msg);
        dataRegFreeStack.push(argreg);

        if(this->lex->peekLexeme().getText() == ",") { // If next lexeme is a comma, just blindly eat it
            this->lex->getNextLexeme();
        }
    }
    paren = this->lex->getNextLexeme(); // Eat the close paren


    // Allocate a register for the return value. This is where this factor's output will be stored
    retvaldreg = dataRegFreeStack.top();
    dataRegFreeStack.pop();
    dataRegStatementStack.push(retvaldreg);

    // Stash the value in D0 temporarily in a different data reg so the return value doesn't overwrite it.
    stashdreg = dataRegFreeStack.top();
    dataRegFreeStack.pop();
//        std::cerr << "allocating register " << dreg << "\n";
    // Only stash D0 if its value is currently in use.
    if(retvaldreg != "D0") {
        snprintf(msg, sizeof(msg), "    MOVE.L D0,%s", stashdreg.c_str());
        emit(msg);
    }

    snprintf(msg, sizeof(msg), "    BSR %s", l.getText().c_str());
    emit(msg);
    if(argument_stack_space_size > 0) {
        snprintf(msg, sizeof(msg), "    ADDA.L #%d,A7", argument_stack_space_size); // Collapse the function call stack frame
        emit(msg);
    }

    snprintf(msg, sizeof(msg), "    MOVE.L D0,%s", retvaldreg.c_str());
    emit(msg);
    dataRegFreeStack.push(stashdreg);
    if(retvaldreg != "D0") {
        snprintf(msg, sizeof(msg), "    MOVE.L %s,D0", stashdreg.c_str());
        emit(msg);
    }
}


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
    unsigned int stack_frame_pos = 8; // first variable on the stack frame will start right above the return address at A6+8
    std::map<std::string, identifier*> symbolTable;

    lexeme returntype =  lex->getNextLexeme(); // Get the function's return type
    lexeme funcname = lex->getNextLexeme(); // Get function name

    snprintf(buf, sizeof(buf), "%s:", funcname.getText().c_str());
    emit(buf);

    // Read function arguments
    lexeme arglex = lex->getNextLexeme(); // Get open paren
    do {
        if(this->lex->peekLexeme().getType() == LEXEME_TYPE_PARENTHESES) {
            break;
        }

        std::string newident = declaration(symbolTable, LEXEME_TYPE_COMMA | LEXEME_TYPE_PARENTHESES);
        symbolTable[newident]->setStackFramePosition(stack_frame_pos);
        stack_frame_pos += symbolTable[newident]->getNumBytes();

        if(debuglevel > 1) {
            // If the user has enabled debugging, we will print the variables locations on the stack frame
            char spaces[50];
            unsigned int k;
            memset(spaces, 0, sizeof(spaces));

            for(k = 0; k < 30-strlen(newident.c_str()); k++) {
                strcat(spaces, " ");
            }
            snprintf(buf, sizeof(buf), "%s |--------------------------------|", COMMENT_STRING);
            emit (buf);
            snprintf(buf, sizeof(buf), "%s | %s%s |  FP + %d", COMMENT_STRING, newident.c_str(), spaces, symbolTable[newident]->getStackFramePosition());
            emit (buf);
        }

        arglex = lex->peekLexeme(); // Read the next lexeme to decide if we need to break out of the loop

    // parser::declaration will eat the close parentheses `)' at the end of the function arg list. We are looking for an open brace to terminate this loop
    } while(arglex.getType() != LEXEME_TYPE_OPENBRACE);
    block(symbolTable);
}


/*
 * block
 *
 * Handles a block of statements enclosed by `{' and `}'
 *
 *
 *
 */
void parser::block(std::map<std::string, identifier*>&symbolTable) {
    int totalBytesInStackFrame = 0;
    char buf[200];
//    std::map<std::string,identifier*> symbolTable;

    lexeme l = lex->peekLexeme();

    // Expected: open brace
    if(l.getType() != LEXEME_TYPE_OPENBRACE) {
        std::cerr << "[block] got " << l.getType() << std::endl;
        this->error("Expected: `{'.");
    }
    // Eat the open brace
    l = lex->getNextLexeme();

    if(debuglevel > 1) {
        snprintf(buf, sizeof(buf), "%s |--------------------------------|", COMMENT_STRING);
        emit(buf);
        snprintf(buf, sizeof(buf), "%s | RETURN ADDRESS                 |", COMMENT_STRING);
        emit(buf);
        snprintf(buf, sizeof(buf), "%s |--------------------------------|", COMMENT_STRING);
        emit(buf);
        snprintf(buf, sizeof(buf), "%s | FP                             | <-- A6", COMMENT_STRING);
        emit(buf);
        snprintf(buf, sizeof(buf), "%s |--------------------------------|", COMMENT_STRING);
        emit(buf);
    }
    // Process declarations
    while((this->lex->peekLexeme().getType() == LEXEME_TYPE_KEYWORD) && (this->lex->peekLexeme().getSubtype() == KEYWORD_TYPE_INT)) {
        // Found a declaration
        std::string newident = this->declaration(symbolTable, LEXEME_TYPE_SEMICOLON);
        totalBytesInStackFrame += symbolTable[newident]->getNumBytes();
        symbolTable[newident]->setStackFramePosition(-totalBytesInStackFrame);

        if(debuglevel > 1) {
            // If the user has enabled debugging, we will print the variables locations on the stack frame
            char spaces[50];
            unsigned int k;
            memset(spaces, 0, sizeof(spaces));

            for(k = 0; k < 30-strlen(newident.c_str()); k++) {
                strcat(spaces, " ");
            }
            snprintf(buf, sizeof(buf), "%s | %s%s |  FP - %d",COMMENT_STRING, newident.c_str(), spaces, -symbolTable[newident]->getStackFramePosition());
            emit (buf);
            snprintf(buf, sizeof(buf), "%s |--------------------------------|", COMMENT_STRING);
            emit(buf);
        }
    }
#if 0
    if(debuglevel > 1) {
        snprintf(buf, sizeof(buf), "%s |--------------------------------|", COMMENT_STRING);
        emit(buf);
    }
#endif
    //////////////////////////////////////////
    // Iterate thru the declarations. Count the total size of the stack frame to be created, and mark the location of each identifier on the stack.
#if 0
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
#endif
    std::string linkinstr("    LINK A6,#");
    linkinstr.append(std::to_string(-totalBytesInStackFrame));
    emit(linkinstr);
    emit((char*)"    MOVEM.L D1-D7/A0-A5,-(A7)");
    // Done iterating thru declarations
    /////////////////////////////////////////

    // Keep eating statements from the block until we encounter a close brace `}'
    while(this->lex->peekLexeme().getType() != LEXEME_TYPE_CLOSEBRACE) {
//        std::cerr << "Next lexeme type is " << this->lex->peekLexeme().getType() << "\n";
        this->statement(symbolTable); // Process statement
    }

    emit((char*)"    MOVEM.L (A7)+,D1-D7/A0-A5");
    emit((char*)"    UNLK A6");
    emit((char*)"    RTS");
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
std::string parser::declaration(std::map<std::string, identifier*>&symbolTable, int declaration_terminator) {

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
    // TODO: error-check the terminator
    if(this->lex->peekLexeme().getType() & declaration_terminator) {
        std::cerr << "[parser::declaration] Got semicolon \"" << this->lex->peekLexeme().getText() << "\" type = "<< this->lex->peekLexeme().getType() << std::endl;
        l = lex->getNextLexeme();
    } else {
        std::cerr << "[parser::declaration] DIDN'T GET SEMICOLON!! GOT " << this->lex->peekLexeme().getType() << " INSTEAD!!" << std::endl;
        char msg[200];
        snprintf(msg, 200, "Expected semicolon at end of declaration. Got `%s' instead.", this->lex->peekLexeme().getText().c_str());
        error(msg);
    }
    return identifiername.getText();
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

    lexeme l = lex->getNextLexeme();

    // This if block deals with statements of the form:
    //
    //      ident = expr;
    //
    // The first bit processes the identifier by finding it in the symbol table.
    if(l.getType() == LEXEME_TYPE_IDENT) {
        if( (this->lex->peekLexeme().getType() == LEXEME_TYPE_PARENTHESES) && (this->lex->peekLexeme().getText() == "(")) {
            do_function_call(l, symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack);

            lexeme paren = this->lex->getNextLexeme(); // Eat the semicolon
            std::cerr << "[statement] Got lexeme \"" << paren.getText() << "\" after function call (expecting semicolon).\n";
            // do_function_call saves the function's return value in a register
            // that gets pushed on dataRegStatementStack. Since we don't need
            // that value (if we got here, we are calling a function and
            // disgarding its return value), we will put the return value
            // register back on the free stack.
            std::string allocdreg = dataRegStatementStack.top();
            dataRegStatementStack.pop();
            dataRegFreeStack.push(allocdreg);
        } else {
            // Assignment statement
            // Look up the identifier in the symbol table for this block.
            identifier *id = symbolTable[l.getText()];
            if(id == NULL) {
                // Check to make sure the symbol is in our symbol table.
                char msg[100];
                snprintf(msg, sizeof(msg), "Identifier `%s' undeclared.", l.getText().c_str());
                this->error(msg);
            }
            lexeme idlexeme = l; // Save the identifier lexeme
            l = this->lex->getNextLexeme(); // Eat the `='
            if(l.getType() != LEXEME_TYPE_ASSIGNMENTOP) {
                std::cerr << "[parser::statement] Got lexeme type " << l.getType() << std::endl;
                this->error("Expected `=' in assignment");
            }
            // Process the expression
            this->expression(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack);

            // Store the result on the stack at the location allocated to the identifier
            char msg[200];
            snprintf(msg, sizeof(msg), "    MOVE.L %s,%d(A6)", dataRegStatementStack.top().c_str(), id->getStackFramePosition());
            emit(msg);
        }
    } else if ((l.getType() == LEXEME_TYPE_KEYWORD) && (l.getSubtype() == KEYWORD_TYPE_RETURN)) {
        // Handle return statements -- first, process the expression in the
        // return statement. The result of the expression will be stored on the
        // top of dataRegStatementStack. We will put this value into D0
        char msg[200];
        lexeme keywordlexeme = l; // Eat the return keyword
        this->expression(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack);

        // Get the result of the expression into D0
        snprintf(msg, sizeof(msg), "    MOVE.L %s,D0", dataRegStatementStack.top().c_str());
        emit(msg);
        dataRegStatementStack.pop(); // Remove the result of the expression from the DataRegStatementStack
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
 * expression
 *
 * Handles expressions. The result of the expression is stored in dataRegStatementStack.top().
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
            snprintf(msg,sizeof(msg), "    ADD %s,%s", op1.c_str(), op2.c_str());
        } else if (sign_lexeme.getText() == "-") {
            snprintf(msg,sizeof(msg), "    SUB %s,%s", op1.c_str(), op2.c_str());
        } else {
            snprintf(msg, sizeof(msg), "Error: expected `+' or `-' in expression. Got \"%s\"", sign_lexeme.getText().c_str());
            error(msg);
        }
        emit(msg);
        dataRegFreeStack.push(op1); // Reclaim one of the data registers
        dataRegStatementStack.push(op2); // Put the result register on the statement stack

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
            snprintf(msg,sizeof(msg), "    MULS %s,%s", op1.c_str(), op2.c_str());
        } else if (mulop_lexeme.getText() == "/") {
            snprintf(msg,sizeof(msg), "    DIVS %s,%s", op1.c_str(), op2.c_str());
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
        snprintf(buf, sizeof(buf), "    NEG.L %s", regname.c_str());
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
        snprintf(msg, sizeof(msg), "    MOVE.L #%d,%s", l.getValue(), dreg.c_str());
        emit(msg);

        break;
    case LEXEME_TYPE_IDENT:
//        std::cerr << "[parser::factor] Found identifier\n";

        if((this->lex->peekLexeme().getType() == LEXEME_TYPE_PARENTHESES) && (this->lex->peekLexeme().getText() == "(")) {
            
            do_function_call(l, symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack);
        } else {
            id = symbolTable[l.getText()];
            if(id == NULL) {
                // Check to make sure the symbol is in our symbol table.
                snprintf(msg, sizeof(msg), "Identifier `%s' undeclared.", this->lex->peekLexeme().getText().c_str());
                this->error(msg);
            }
            dreg = dataRegFreeStack.top();
            dataRegFreeStack.pop();
            dataRegStatementStack.push(dreg);
    //        std::cerr << "allocating register " << dreg << "\n";
            snprintf(msg, sizeof(msg), "    MOVE.L %d(A6),%s", id->getStackFramePosition(), dreg.c_str());
            emit(msg);
        }
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
