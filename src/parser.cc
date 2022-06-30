


#include "parser.h"
#include <map>
#include <sstream>
#include <cstring>

extern unsigned int debuglevel;


void parser::do_function_call(lexeme l, std::map<std::string, identifier*>&symbolTable, std::stack<std::string>&dataRegFreeStack, std::stack<std::string>&dataRegStatementStack, std::stack<std::string>&addrRegFreeStack, std::stack<std::string>&addrRegStatementStack) {
    // Size (in bytes) of arguments passed to the function. We will use
    // this later to collapse the stack frame after the function returns
    unsigned int argument_stack_space_size = 0;
    std::string retvaldreg, stashdreg; // Data register name used to process this factor
    // Function Call
    std::string funcname = l.getText().c_str();
    lexeme paren = this->lex->getNextLexeme(); // Eat the open paren


    // Copped from:
    // https://stackoverflow.com/questions/5419356/redirect-stdout-stderr-to-a-string
    std::stringstream tmp_cout_buf;
    std::string function_call_instructions;
    std::streambuf * old = std::cout.rdbuf(tmp_cout_buf.rdbuf());

    std::string text = tmp_cout_buf.str(); // text will now contain "Bla\n"

    // Process function arguments
    while(this->lex->peekLexeme().getType() == LEXEME_TYPE_IDENT) {
        // expression
        this->expression(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack);

        std::string argreg = dataRegStatementStack.top();
        dataRegStatementStack.pop();
        argument_stack_space_size += 4; // Increment stack frame size by the size of the arg we are passing
        emit("    MOVE.L " + argreg + ",-(A7)");
        dataRegFreeStack.push(argreg);

        function_call_instructions = tmp_cout_buf.str() + function_call_instructions;
        tmp_cout_buf.str("");

        if(this->lex->peekLexeme().getText() == ",") { // If next lexeme is a comma, just blindly eat it
            this->lex->getNextLexeme();
        }
    }
    std::cout.rdbuf(old);

    std::cout << function_call_instructions;

    paren = this->lex->getNextLexeme(); // Eat the close paren


    // Allocate a register for the return value. This is where this factor's output will be stored
    retvaldreg = dataRegFreeStack.top();
    dataRegFreeStack.pop();
    dataRegStatementStack.push(retvaldreg);

    // Stash the value in D0 temporarily in a different data reg so the return value doesn't overwrite it.
    stashdreg = dataRegFreeStack.top();
    dataRegFreeStack.pop();
    // Only stash D0 if its value is currently in use.
    if(retvaldreg != "D0") {
        emit("    MOVE.L D0," + stashdreg);
    }

    emit("    BSR " + l.getText());
    if(argument_stack_space_size > 0) {
        emit("    ADDA.L #" + std::to_string(argument_stack_space_size) + ",A7");
    }

    emit("    MOVE.L D0," + retvaldreg);
    dataRegFreeStack.push(stashdreg);
    if(retvaldreg != "D0") {
        emit("    MOVE.L " + stashdreg + ",D0");
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
    next_label_index = 0;
}


std::string parser::generateNewLabel() {
    std::string newLabel("L" + std::to_string(this->next_label_index));
    next_label_index++;
    return newLabel;
}

void parser::error(const char *msg) {
    std::cerr << *input_file_name << " Line " << lex->getCurrLineNumber() << " Col " << lex->getCurrColumnNumber() << ": " << msg << "\n";
    exit(1);
}

void parser::error(std::string msg) {
    std::cerr << *input_file_name << " Line " << lex->getCurrLineNumber() << " Col " << lex->getCurrColumnNumber() << ": " << msg << "\n";
    exit(1);
}

int parser::function() {
    unsigned int stack_frame_pos = 8; // first variable on the stack frame will start right above the return address at A6+8
    std::map<std::string, identifier*> symbolTable;

    lexeme returntype =  lex->getNextLexeme(); // Get the function's return type
    lexeme funcname = lex->getNextLexeme(); // Get function name

    emit(funcname.getText() + ":");

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
            emit(std::string(COMMENT_STRING) + std::string(" |---------------------------------|"));
            emit(std::string(COMMENT_STRING) + " | " + newident + " " + std::string(spaces) + " | FP + " + std::to_string(symbolTable[newident]->getStackFramePosition()));
        }

        arglex = lex->peekLexeme(); // Read the next lexeme to decide if we need to break out of the loop

    // parser::declaration will eat the close parentheses `)' at the end of the function arg list. We are looking for an open brace to terminate this loop
    } while(arglex.getType() != LEXEME_TYPE_OPENBRACE);

    if(debuglevel > 1) {
        emit(std::string(COMMENT_STRING) + std::string(" |---------------------------------|"));
        emit(std::string(COMMENT_STRING) + std::string(" | RETURN ADDRESS                  |"));
    }

    block(symbolTable, 1);
    emit((char*)"    RTS");
    if(lex->peekLexeme().getType() == LEXEME_TYPE_EOF){
        return -1;
    } else {
        return 0;
    }
}


/*
 * block
 *
 * Handles a block of statements enclosed by `{' and `}'
 *
 *
 *
 */
void parser::block(std::map<std::string, identifier*>&symbolTable, int createStackFrame) {
    int totalBytesInStackFrame = 0;

    lexeme l = lex->peekLexeme();

    // Expected: open brace
    if(l.getType() != LEXEME_TYPE_OPENBRACE) {
        std::cerr << "[block] got \"" << l.getText() << "\"\n";
        this->error("Expected: `{'.");
    }
    // Eat the open brace
    l = lex->getNextLexeme();

    if(createStackFrame) {
        if(debuglevel > 1) {
            emit(std::string(COMMENT_STRING) + std::string(" |---------------------------------|"));
            emit(std::string(COMMENT_STRING) + std::string(" | FP                              | <-- A6"));
            emit(std::string(COMMENT_STRING) + std::string(" |---------------------------------|"));
        }
        // Process declarations
        while((this->lex->peekLexeme().getType() == LEXEME_TYPE_KEYWORD) && 
              ((this->lex->peekLexeme().getSubtype() == KEYWORD_TYPE_INT) ||
              (this->lex->peekLexeme().getSubtype() == KEYWORD_TYPE_CHAR))) {
            // Found a declaration
            std::string newident = this->declaration(symbolTable, LEXEME_TYPE_SEMICOLON);
            totalBytesInStackFrame += symbolTable[newident]->getNumBytes();

            // Make sure to align multibyte datatypes to even addresses
            if((totalBytesInStackFrame % 2) != 0 && (symbolTable[newident]->getNumBytes() > 1)) {
                totalBytesInStackFrame++;
            }
            symbolTable[newident]->setStackFramePosition(-totalBytesInStackFrame);

            if(debuglevel > 1) {
                // If the user has enabled debugging, we will print the variables locations on the stack frame
                char spaces[50];
                unsigned int k;
                memset(spaces, 0, sizeof(spaces));

                for(k = 0; k < 30-strlen(newident.c_str()); k++) {
                    strcat(spaces, " ");
                }
                emit(std::string(COMMENT_STRING) + std::string(" | " + newident + std::string(spaces) + "  |  FP - " + std::to_string(-symbolTable[newident]->getStackFramePosition())));
                emit(std::string(COMMENT_STRING) + std::string(" |---------------------------------|"));
            }
        }

        // Make sure to align the stack pointer to an even address
        if((totalBytesInStackFrame % 2) != 0) {
            totalBytesInStackFrame++;
        }
        std::string linkinstr("    LINK A6,#" + std::to_string(-totalBytesInStackFrame));
        emit(linkinstr);
        emit((char*)"    MOVEM.L D1-D7/A0-A5,-(A7)");
    }
    // Done iterating thru declarations
    /////////////////////////////////////////

    // Keep eating statements from the block until we encounter a close brace `}'
    while(this->lex->peekLexeme().getType() != LEXEME_TYPE_CLOSEBRACE) {
        this->statement(symbolTable); // Process statement
    }

    if(createStackFrame) {
        emit((char*)"    MOVEM.L (A7)+,D1-D7/A0-A5");
        emit((char*)"    UNLK A6");
    }
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


void parser::emit(std::string s) {
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
        case KEYWORD_TYPE_CHAR:
//            std::cerr << "found keyword char\n";
            id->setType(IDENTIFIER_TYPE_INTEGER);
            id->setNumBytes(1);
            break;
        case KEYWORD_TYPE_INT:
//            std::cerr << "found keyword int\n";
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

    // Add the new identifier to the symbol table.
    lexeme identifiername = this->lex->getNextLexeme();

    // Check if there's a * before the identifier name. If so, this declaration is a pointer.
    if (identifiername.getType() == LEXEME_TYPE_MULOP) {
        id->setNumBytes(4); // Re-set the number of bytes this variable will take in case it was set incorrectly above.
        id->setType(IDENTIFIER_TYPE_POINTER); // Re-set the type of this identifier
        identifiername = this->lex->getNextLexeme();
    }

    symbolTable.insert(std::pair<std::string,identifier*>(identifiername.getText(),id) );

    // Eat the semicolon lexeme
    if(this->lex->peekLexeme().getType() & declaration_terminator) {
        l = lex->getNextLexeme();
    } else {
//        char msg[200];
//        snprintf(msg, 200, "Expected semicolon at end of declaration. Got `%s' instead.", this->lex->peekLexeme().getText().c_str());
        error("Expected semicolon at end of declaration. Got `" +this->lex->peekLexeme().getText() +"' instead.");
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
                this->error("Identifier `" + l.getText() + "' undeclared.");
            }
            lexeme idlexeme = l; // Save the identifier lexeme
            l = this->lex->getNextLexeme(); // Eat the `='
            if(l.getType() != LEXEME_TYPE_ASSIGNMENTOP) {
                std::cerr << "[parser::statement] Got lexeme type " << l.getType() << std::endl;
                this->error("Expected `=' in assignment");
            }
            // Process the expression
            unsigned int expressionSize = this->expression(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack);

            // Store the result on the stack at the location allocated to the identifier

            std::string opsz = getSizeSuffix(expressionSize);
            emit("    MOVE." + opsz + " " + dataRegStatementStack.top() + "," + std::to_string(id->getStackFramePosition()) + "(A6)");
        }
    } else if ((l.getType() == LEXEME_TYPE_KEYWORD) && (l.getSubtype() == KEYWORD_TYPE_RETURN)) {
        // Handle return statements -- first, process the expression in the
        // return statement. The result of the expression will be stored on the
        // top of dataRegStatementStack. We will put this value into D0
        lexeme keywordlexeme = l; // Eat the return keyword
        unsigned int expressionSize = this->expression(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack);

        // Get the result of the expression into D0
        if(expressionSize != 4) {
            // If the expression is not a longword, clear the D0 return register so the high-order bits are zeros
            emit("    CLR.L D0");
        }
        emit("    MOVE.L " + dataRegStatementStack.top() + ",D0");
        dataRegStatementStack.pop(); // Remove the result of the expression from the DataRegStatementStack
    } else if ((l.getType() == LEXEME_TYPE_KEYWORD) && (l.getSubtype() == KEYWORD_TYPE_WHILE)) {
        // While loop is organized as follows:
        //
        // L1:    ; bexpr_comparison_label
        //    <while loop comparison stuff/bexpression implementation>
        // L2:    ; bexpr_true_label
        //    <while loop body>
        //    BRA L1
        // L3:    ; bexpr_false_label
        //    <while loop done>
        //
        // The bexpression needs 2 labels: one in case the comparison is true,
        // one in case the comparison is false. If the comparison is true, we
        // will branch to L2, which will cause us to execute the body of the
        // while loop. If the comparison is false, we will branch to L3, causing
        // us to break out of the loop.
        if(this->lex->getNextLexeme().getText() != "(") {
            error("Error: expected `(' after while.");
        }
        std::string bexpr_comparison_label = this->generateNewLabel();
        std::string bexpr_true_label = this->generateNewLabel();
        std::string bexpr_false_label = this->generateNewLabel();
        emit(bexpr_comparison_label + ":");
        this->bexpression(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack, bexpr_true_label, bexpr_false_label);
        if(this->lex->getNextLexeme().getText() != ")") {
            error("Error: expected `)' at end of if block.");
        }
        emit(bexpr_true_label + ":");
        block(symbolTable,0);
        emit("    BRA " + bexpr_comparison_label);
        emit(bexpr_false_label + ":");
    } else if ((l.getType() == LEXEME_TYPE_KEYWORD) && (l.getSubtype() == KEYWORD_TYPE_IF)) {
        if(this->lex->getNextLexeme().getText() != "(") {
            error("Error: expected `(' after if.");
        }
        std::string bexpr_true_label = this->generateNewLabel();
        std::string bexpr_false_label = this->generateNewLabel();
        this->bexpression(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack, bexpr_true_label, bexpr_false_label);
        if(this->lex->getNextLexeme().getText() != ")") {
            error("Error: expected `)' at end of if block.");
        }
        emit(bexpr_true_label + ":");
        block(symbolTable,0);
        emit(bexpr_false_label + ":");
    } else {
        this->error("Expected: identifier at beginning of statement");
    }
    // Eat the semicolong lexeme...
    if(this->lex->peekLexeme().getType() == LEXEME_TYPE_SEMICOLON) {
        l = lex->getNextLexeme();
    }
//    std::cout << "Done processing statement...\n\n";
}

// NOTE: Productions for logical expressions are on page 402 of the dragon book
void parser::bexpression(std::map<std::string, identifier*>&symbolTable, std::stack<std::string>&dataRegFreeStack, std::stack<std::string>&dataRegStatementStack, std::stack<std::string>&addrRegFreeStack, std::stack<std::string>&addrRegStatementStack, std::string true_label, std::string false_label) {
//    this->bterm(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack);

    // Temporary: Put relation code in here.
//    std::cerr << "[parser::bexpression] Next lexeme is \"" << this->lex->peekLexeme().getText() << "\"\n";

    // Process the first factor
    this->factor(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack); // Process the factor
    
    // Get the relative operation
    lexeme relop = this->lex->getNextLexeme();

    // Process the second factor
    this->factor(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack); // Process the factor

    // Get the factors to compare from the dataRegStatementStack. They were put
    // there by the two previous calls to this->factor()
    std::string firstfactorreg = dataRegStatementStack.top();
    dataRegStatementStack.pop();

    std::string secondfactorreg = dataRegStatementStack.top();
    dataRegStatementStack.pop();

    emit("    CMP " + firstfactorreg + "," + secondfactorreg);

    switch(relop.getSubtype()) {
    case COMPARISON_TYPE_LESS_THAN:
        emit("    BLT " + true_label);
        emit("    BRA " + false_label);
        break;
    case COMPARISON_TYPE_GREATER_THAN:
        emit("    BGT " + true_label);
        emit("    BRA " + false_label);
        break;
    case COMPARISON_TYPE_LESS_OR_EQUAL:
        emit("    BLE " + true_label);
        emit("    BRA " + false_label);
        break;
    case COMPARISON_TYPE_GREATER_OR_EQUAL:
        emit("    BGE " + true_label);
        emit("    BRA " + false_label);
        break;
    case COMPARISON_TYPE_EQUAL:
        emit("    BEQ " + true_label);
        emit("    BRA " + false_label);
        break;
    case COMPARISON_TYPE_NOT_EQUAL:
        emit("    BNE " + true_label);
        emit("    BRA " + false_label);
        break;
        }

}


void parser::bterm(std::map<std::string, identifier*>&symbolTable, std::stack<std::string>&dataRegFreeStack, std::stack<std::string>&dataRegStatementStack, std::stack<std::string>&addrRegFreeStack, std::stack<std::string>&addrRegStatementStack) {

    this->notfactor(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack);
}

void parser::notfactor(std::map<std::string, identifier*>&symbolTable, std::stack<std::string>&dataRegFreeStack, std::stack<std::string>&dataRegStatementStack, std::stack<std::string>&addrRegFreeStack, std::stack<std::string>&addrRegStatementStack) {

    this->bfactor(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack);
}

void parser::bfactor(std::map<std::string, identifier*>&symbolTable, std::stack<std::string>&dataRegFreeStack, std::stack<std::string>&dataRegStatementStack, std::stack<std::string>&addrRegFreeStack, std::stack<std::string>&addrRegStatementStack) {

    this->relation(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack);
}

void parser::relation(std::map<std::string, identifier*>&symbolTable, std::stack<std::string>&dataRegFreeStack, std::stack<std::string>&dataRegStatementStack, std::stack<std::string>&addrRegFreeStack, std::stack<std::string>&addrRegStatementStack) {

}

/*
 * expression
 *
 * Handles expressions. The result of the expression is stored in dataRegStatementStack.top(). Returns the size of the expression (in bytes).
 *
 */
unsigned int parser::expression(std::map<std::string, identifier*>&symbolTable, std::stack<std::string>&dataRegFreeStack, std::stack<std::string>&dataRegStatementStack, std::stack<std::string>&addrRegFreeStack, std::stack<std::string>&addrRegStatementStack) {
    // expression    -> term [(`+' | `-') term]*
    // Process the first term
    unsigned int expressionSize = 0;
    expressionSize = this->term(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack); // Eat the identifier

    // Check if we have an addop after the first term.
    while(this->lex->peekLexeme().getType() == LEXEME_TYPE_ADDOP) {
        lexeme sign_lexeme = this->lex->getNextLexeme(); // Get the sign (`+' or `-')
        unsigned int termSize = this->term(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack); // Eat the identifier

        // Promote the size of this expression to max(expression sz, term sz)
        expressionSize = termSize > expressionSize ? termSize : expressionSize;

        std::string opsz = getSizeSuffix(expressionSize);
        // Now add or subtract the terms
        char msg[200];
        std::string op1, op2;
        op1 = dataRegStatementStack.top();
        dataRegStatementStack.pop();
        op2 = dataRegStatementStack.top();
        dataRegStatementStack.pop();
        if(sign_lexeme.getText() == "+") {
            emit("    ADD."+ opsz + " "+ op1 + "," + op2);
        } else if (sign_lexeme.getText() == "-") {
            emit("    SUB." + opsz +" "+ op1 + "," + op2);
        } else {
            snprintf(msg, sizeof(msg), "Error: expected `+' or `-' in expression. Got \"%s\"", sign_lexeme.getText().c_str());
            error(msg);
        }
        dataRegFreeStack.push(op1); // Reclaim one of the data registers
        dataRegStatementStack.push(op2); // Put the result register on the statement stack
    }
    return expressionSize;
}

/*
 *
 *
 *
 *
 */
unsigned int parser::term(std::map<std::string, identifier*>&symbolTable, std::stack<std::string>&dataRegFreeStack, std::stack<std::string>&dataRegStatementStack, std::stack<std::string>&addrRegFreeStack, std::stack<std::string>&addrRegStatementStack) {
    unsigned int termSize = 0;
    //term          -> signedfactor [(`*' | `/') factor]*
    termSize = this->signedfactor(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack); // Eat the identifier

    // Handle multiple factors following the first signed factor
    while(this->lex->peekLexeme().getType() == LEXEME_TYPE_MULOP) {
        lexeme mulop_lexeme = this->lex->getNextLexeme(); // Get the operation (`*' or `/')
        unsigned int factorSize = this->factor(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack); // Process the factor

        termSize = termSize > factorSize ? termSize : factorSize;

        // Now multiply or divide the terms
        char msg[200];
        std::string op1, op2;
        op1 = dataRegStatementStack.top();
        dataRegStatementStack.pop();
        op2 = dataRegStatementStack.top();
        dataRegStatementStack.pop();

        if(mulop_lexeme.getText() == "*") {
            emit("    MULS "+ op1 + "," + op2);
        } else if (mulop_lexeme.getText() == "/") {
            emit("    DIVS "+ op1 + "," + op2);
        } else {
            snprintf(msg, sizeof(msg), "Error: expected `*' or `/' in term. Got \"%s\"", mulop_lexeme.getText().c_str());
            error(msg);
        }
        dataRegFreeStack.push(op1); // Reclaim one of the data registers
        dataRegStatementStack.push(op2);
    }
    return termSize;
}

unsigned int parser::signedfactor(std::map<std::string, identifier*>&symbolTable, std::stack<std::string>&dataRegFreeStack, std::stack<std::string>&dataRegStatementStack, std::stack<std::string>&addrRegFreeStack, std::stack<std::string>&addrRegStatementStack) {
    unsigned int factorSize = 0;
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
    
    factorSize = this->factor(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack);
    
    if(sign.getText() == "-") {
        // If the factor has a `-' sign in front of it, negate the value.
        std::string regname =  dataRegStatementStack.top();
        emit("    NEG.L " + regname);
    }
    return factorSize;
}
/*
 * factor
 *
 * Processes a factor of the form:
 *
 *  factor        -> constant | identifier | `(' <expression> `)'
 *                 | funcname ( [identifier ,]* ) ;
 *
 * The result of the factor is stored in a data register, which is saved on top
 * of the dataRegStatementStack. Returns the size (in bytes) of the factor.
 *
 */
unsigned int parser::factor(std::map<std::string, identifier*>&symbolTable, std::stack<std::string>&dataRegFreeStack, std::stack<std::string>&dataRegStatementStack, std::stack<std::string>&addrRegFreeStack, std::stack<std::string>&addrRegStatementStack) {
    unsigned int factorSize = 0;
    lexeme l;
    identifier *id;
    std::string dreg; // Data register name used to process this factor
    char msg[200];

    l = this->lex->getNextLexeme(); // Get the next lexeme. Should be a constant, identifier, or open parentheses
    switch(l.getType()) {
    case LEXEME_TYPE_INTEGER:
        factorSize = 4;
        dreg = dataRegFreeStack.top();
        dataRegFreeStack.pop();
        dataRegStatementStack.push(dreg);
        emit("    MOVE.L #" + std::to_string(l.getValue()) + "," + dreg);

        break;
    case LEXEME_TYPE_IDENT:
        if((this->lex->peekLexeme().getType() == LEXEME_TYPE_PARENTHESES) && (this->lex->peekLexeme().getText() == "(")) {
            
            do_function_call(l, symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack);
            factorSize = 4;
        } else {
            id = symbolTable[l.getText()];
            if(id == NULL) {
                // Check to make sure the symbol is in our symbol table.
                snprintf(msg, sizeof(msg), "Identifier `%s' undeclared.", this->lex->peekLexeme().getText().c_str());
                this->error(msg);
            }
            std::string szsuffix = getSizeSuffix(id->getNumBytes());

            dreg = dataRegFreeStack.top();
            dataRegFreeStack.pop();
            dataRegStatementStack.push(dreg);
            emit("    MOVE." + szsuffix + " " + std::to_string(id->getStackFramePosition()) + "(A6),"+dreg);
        }
        break;
    case LEXEME_TYPE_PARENTHESES:
        // First, ensure that we got an open paren, not a closed.
        if(l.getText() != "(") {
            snprintf(msg, sizeof(msg), "Error: Expected `(' before `%s'", this->lex->peekLexeme().getText().c_str());
            this->error(msg);
        }
        // Process the expression
        factorSize = this->expression(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack);
        if(this->lex->peekLexeme().getText() != ")") {
            snprintf(msg, sizeof(msg), "Error: Expected `)' at the end of the expression. Got `%s' instead.", this->lex->peekLexeme().getText().c_str());
            this->error(msg);
        } else {
            l = this->lex->getNextLexeme(); // Eat the close paren
        }
        break;
    case LEXEME_TYPE_MULOP:
        if(l.getText() == "*") {
            std::cerr << "[parser::factor] found pointer\n";
            pointer(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack);
        } else {
            error("Eroor: Trying to process pointer in parser::factor and got " + l.getText());
        }
        break;
    default:
        snprintf(msg, sizeof(msg), "Error processing factor. Expected integer constant or identifier. Got `%s'", l.getText().c_str());
        error(msg);
        break;
    }
    return factorSize;
}

void parser::pointer (std::map<std::string, identifier*>&symbolTable, std::stack<std::string>&dataRegFreeStack, std::stack<std::string>&dataRegStatementStack, std::stack<std::string>&addrRegFreeStack, std::stack<std::string>&addrRegStatementStack) {
    lexeme l;
    identifier *id;
    std::string dreg; // Data register name used to process this factor
    l = this->lex->getNextLexeme(); // Get the next lexeme. Should be a constant, identifier, or open parentheses

    std::cerr << "[parser::pointer] got lexeme " + l.getText() << "\n";
    switch(l.getType()) {
    case LEXEME_TYPE_IDENT:
        if((this->lex->peekLexeme().getType() == LEXEME_TYPE_PARENTHESES) && (this->lex->peekLexeme().getText() == "(")) {
            
            do_function_call(l, symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack);
        } else {
            id = symbolTable[l.getText()];
            if(id == NULL) {
                // Check to make sure the symbol is in our symbol table.
//                snprintf(msg, sizeof(msg), "Identifier `%s' undeclared.", this->lex->peekLexeme().getText().c_str());
                this->error("Identifier `" + this->lex->peekLexeme().getText() + "' undeclared.");
            }
            dreg = dataRegFreeStack.top();
            dataRegFreeStack.pop();
            dataRegStatementStack.push(dreg);
            emit("    MOVE.L " + std::to_string(id->getStackFramePosition()) + "(A6),"+dreg);
        }
        break;
    case LEXEME_TYPE_PARENTHESES:
        // First, ensure that we got an open paren, not a closed.
        if(l.getText() != "(") {
//            snprintf(msg, sizeof(msg), "Error: Expected `(' before `%s'", this->lex->peekLexeme().getText().c_str());
            this->error("Error: Expected `(' before `" + this->lex->peekLexeme().getText() + "'.");
        }
        // Process the expression
        this->expression(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack);
        if(this->lex->peekLexeme().getText() != ")") {
//            snprintf(msg, sizeof(msg), "Error: Expected `)' at the end of the expression. Got `%s' instead.", this->lex->peekLexeme().getText().c_str());
            this->error("Error: Expected `)' at the end of the expression. Got " +  this->lex->peekLexeme().getText() + "' instead.");
        } else {
            l = this->lex->getNextLexeme(); // Eat the close paren
        }
        break;
    case LEXEME_TYPE_MULOP:
        if(l.getText() == "*") {
            std::cerr << "[parser::factor] found pointer\n";
            pointer(symbolTable, dataRegFreeStack, dataRegStatementStack, addrRegFreeStack, addrRegStatementStack);
        } else {
            error("Eroor: Trying to process pointer in parser::factor and got " + l.getText());
        }
        break;
    default:
//        snprintf(msg, sizeof(msg), "Error processing factor. Expected integer constant or identifier. Got `%s'", l.getText().c_str());
        error("Error processing factor. Expected integer constant or identifier. Got `" + l.getText() + "'");
        break;

    }
}
std::string parser::getSizeSuffix(unsigned int factorSize) {
    std::string szsuffix;
    if(factorSize == 4) {
        szsuffix = "L";
    } else if(factorSize == 2) {
        szsuffix = "W";
    } else {
        szsuffix = "B";
    }
    return szsuffix;
}
