

#include "lexicalparser.h"
#include <iostream>     // std::cin, std::cout
#include <fstream>      // std::ifstream
#include <cstring>
#include <map>




static std::map<std::string, int> keyword_map = {
{"if", KEYWORD_IF},
{"int", KEYWORD_INT},
{"while", KEYWORD_WHILE}
};

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
    return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r');
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
    this->look = this->is->get();
    if (this->look == EOF) {
        std::cout << "[getChar] GOT EOF!!!\n";
//        exit(0);
    }
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
    lexeme nextLexeme;

    // Skip to the next non-blank line
/*
    while(look == '\r') {
        fin();
    }
*/
    skipWhite();
    if(isAlpha(look)) {
        std::string l = getName();

        auto search = keyword_map.find(l); // Look up the string in the keyword list
        if (search != keyword_map.end()) {
            // Got keyword
            std::cout << "[getNextLexeme] got keyword \"" << l << "\"\n";
        } else {
            // Got identifier
            std::cout << "[getNextLexeme] got identifier \"" << l << "\"\n";
        }
        // Look up the lexeme
    } else if (isDigit(look)) {
        int n = getNum();
        nextLexeme.setType(LEXEME_TYPE_INTEGER);
        std::cout << "[getNextLexeme] got number " << n << "\n";
    } else if (isOp(look)) {
        std::string op = getOp();
        if(op == "+" || op == "-") {
            nextLexeme.setType(LEXEME_TYPE_ADDOP);
        } else if( op == "*" || op == "/") {
            nextLexeme.setType(LEXEME_TYPE_MULOP);
        }
        std::cout << "[getNextLexeme] got operator " << op << "\n";
    } else {
        std::cout << "[getNextLexeme] Got unknown " << look << "\n";
        exit(0);
    }

    return nextLexeme;
}


