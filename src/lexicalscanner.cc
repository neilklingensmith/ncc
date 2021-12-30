

#include "lexicalscanner.h"
#include <iostream>     // std::cin, std::cout
#include <fstream>      // std::ifstream
#include <cstring>
#include <map>




static std::map<std::string, int> keyword_map = {
{"if", KEYWORD_TYPE_IF},
{"int", KEYWORD_TYPE_INT},
{"while", KEYWORD_TYPE_WHILE},
};

void lexicalScanner::Expected(std::string err) {
    std::cerr << err << "\n";
    abort();
}



void lexicalScanner::fin() {
    if (this->look == '\r'){
        getChar();
    }
    if (this->look == '\n') {
        getChar();
    }
}



int lexicalScanner::isWhite(char c) {
    return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r');
}


void lexicalScanner::skipWhite() {
    while (isWhite(this->look)) {
        this->getChar();
    }
}


int lexicalScanner::isAlpha(char c) {
    c = toupper(c);
    if((c >= 'A') && (c <= 'Z')) {
        return 1;
    } else {
        return 0;
    }
}

int lexicalScanner::isDigit(char c) {
    if((c >= '0') && (c <= '9')) {
        return 1;
    } else {
        return 0;
    }
}

int lexicalScanner::isAlNum(char c) {
    return isAlpha(c) | isDigit(c);
}


int lexicalScanner::isAddop(char c) {
    if((c =='+') || (c == '-')) {
        return 1;
    } else {
        return 0;
    }
}


std::string lexicalScanner::getName() {
    std::string name;
    if(!isAlpha(this->look)) {
        Expected("Name");
    }
    while(isAlNum(this->look)){
        name.push_back(this->look);
        this->getChar();
    }
    skipWhite();
    return name;
}

int lexicalScanner::getNum() {
    std::string num;

    if(!isDigit(this->look)) {
        Expected("Integer");
    }
    while(isDigit(this->look)) {
        num.push_back(this->look);
        getChar();
    }
    skipWhite();
    return std::stoi(num);
}

std::string lexicalScanner::getOp() {
    std::string op;

    if(!isOp(look)) {
        Expected("Operator");
    }
    while(isOp(look)) {
        op.push_back(this->look);
        getChar();
    }
    skipWhite();
    return op;
}

void lexicalScanner::getChar() {
    this->look = this->is->get();
    if(this->look == '\n') {
        this->currLineNumber++;
        currColumnNumber = 0;
    }
    if((this->look != '\n') && (this->look != '\r')){
        currColumnNumber++;
    }
    if (this->look == EOF) {
//        std::cout << "[getChar] GOT EOF!!!\n";
//        exit(0);
    }
}

unsigned int lexicalScanner::getCurrColumnNumber() {
    return currColumnNumber;
}


unsigned int lexicalScanner::getCurrLineNumber() {
    return currLineNumber;
}

int lexicalScanner::isOp(char c) {
    return (c == '+') || (c == '-') || (c =='*') || (c == '/') || (c == '<') || (c == '>') || (c == ':') || (c == '=');
}



lexicalScanner::lexicalScanner(char *fname){
    currLineNumber = 1;
    is = new std::ifstream(fname);
    this->fname = new char[strlen(fname)];
    strcpy(this->fname, fname);

    getChar();
}

lexeme lexicalScanner::peekLexeme() {
    return this->currLexeme;
}

lexeme lexicalScanner::getNextLexeme() {


    lexeme lastLexeme = this->currLexeme;

    skipWhite();
    this->currLexeme.setValue(0);
    this->currLexeme.setType(0);
    char looktxt[2] = {(char)look,0};
    currLexeme.setText(looktxt);
    if(isAlpha(look)) {
        this->currLexeme.setText(getName());

        auto search = keyword_map.find(this->currLexeme.getText()); // Look up the string in the keyword list
        if (search != keyword_map.end()) {
            // Got keyword
//            std::cerr << "[getNextLexeme] got keyword \"" << this->currLexeme.getText() << "\"\n";
//            std::cerr << "[getNextLexeme] Found " << search->first << " : " << search->second << std::endl;
            currLexeme.setType(LEXEME_TYPE_KEYWORD);     // Set the lexeme type to keyword
//            std::cerr << "[getNextLexeme] Setting lexeme type to " << currLexeme.getType() << std::endl;
            this->currLexeme.setSubtype(search->second); // Set the type of keyword (IF, WHILE, etc.)
//            std::cerr << "[getNextLexeme] Setting lexeme subtype to " << currLexeme.getSubtype() << std::endl;
        } else {
            // Got identifier
//            std::cerr << "[getNextLexeme] got identifier \"" << this->currLexeme.getText() << "\"\n";
            currLexeme.setType(LEXEME_TYPE_IDENT);
        }
        // Look up the lexeme
    } else if (isDigit(look)) {
        int n = getNum();
        std::string numstr = std::to_string(n);
        currLexeme.setValue(n);
        currLexeme.setText(numstr);
        currLexeme.setType(LEXEME_TYPE_INTEGER);
        //std::cout << "[getNextLexeme] got number " << n << "\n";
    } else if (isOp(look)) {
        std::string op = getOp();
        if(op == "+" || op == "-") {
            currLexeme.setType(LEXEME_TYPE_ADDOP);
        } else if( op == "*" || op == "/") {
            currLexeme.setType(LEXEME_TYPE_MULOP);
        } else if( op == "=") {
            currLexeme.setType(LEXEME_TYPE_ASSIGNMENTOP);
        }
        currLexeme.setText(op);
//        std::cout << "[getNextLexeme] got operator " << op << "\n";
    } else if (look == ';') {
        //std::cout << "Got semicolon. End of statement\n";
        getChar();
        skipWhite();
        currLexeme.setType(LEXEME_TYPE_SEMICOLON);
        currLexeme.setText(";");
    } else if (look == '(' || look == ')') {
        //std::cout << "Got parentheses\n";
        getChar();
        skipWhite();
        currLexeme.setType(LEXEME_TYPE_PARENTHESES);
    } else if (look == '{') {
        //std::cout << "Got open brace\n";
        getChar();
        skipWhite();
        currLexeme.setType(LEXEME_TYPE_OPENBRACE);
    } else if (look == '}') {
        //std::cout << "Got close brace\n";
        getChar();
        skipWhite();
        currLexeme.setType(LEXEME_TYPE_CLOSEBRACE);
    } else {
        //std::cout << "[getNextLexeme] Got unknown " << look << "\n";
        exit(0);
    }

    return lastLexeme;
}


