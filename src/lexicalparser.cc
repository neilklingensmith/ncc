

#include "lexicalparser.h"
#include <iostream>     // std::cin, std::cout
#include <fstream>      // std::ifstream


void lexicalParser::Expected(std::string err) {
    std::cerr << err << "\n";
    abort();
}



void lexicalParser::fin() {
    if (this->look == '\r'){
        getChar();
    }
    if (this->look == '\n') {
        getChar();
    }
}



int lexicalParser::isWhite(char c) {
    return (c == ' ') || (c == '\t');
}


void lexicalParser::skipWhite() {
    while (isWhite(this->look)) {
        this->getChar();
    }
}


int lexicalParser::isAlpha(char c) {
    c = toupper(c);
    if((c >= 'A') && (c <= 'Z')) {
        return 1;
    } else {
        return 0;
    }
}

int lexicalParser::isDigit(char c) {
    if((c >= '0') && (c <= '9')) {
        return 1;
    } else {
        return 0;
    }
}

int lexicalParser::isAlNum(char c) {
    return isAlpha(c) | isDigit(c);
}


int lexicalParser::isAddop(char c) {
    if((c =='+') || (c == '-')) {
        return 1;
    } else {
        return 0;
    }
}


std::string lexicalParser::getName() {
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

int lexicalParser::getNum() {
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

std::string lexicalParser::getOp() {
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

void lexicalParser::getChar() {
    this->is->get(this->look);
}


int lexicalParser::isOp(char c) {
    return (c == '+') || (c == '-') || (c =='*') || (c == '/') || (c == '<') || (c == '>') || (c == ':') || (c == '=');
}



lexicalParser::lexicalParser(char *fname){
    is = new std::ifstream(fname);
    this->fname = new char[strlen(fname)];
    strcpy(this->fname, fname);

    getChar();
}

lexeme lexicalParser::getNextLexeme() {
    lexeme *nextLexeme = new lexeme;

    // Skip to the next non-blank line
    while(look == '\r') {
        fin();
    }

    if(isAlpha(look)) {
        std::string l = getName();
        std::cout << "[getNextLexeme] got \"" << l << "\"\n";
        // Look up the lexeme
    } else if (isDigit(look)) {
        int n = getNum();
        std::cout << "[getNextLexeme] got " << n << "\n";
    }
}
