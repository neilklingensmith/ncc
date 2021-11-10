


#include "lexeme.h"
#include <iostream>     // std::cin, std::cout
#include <fstream>      // std::ifstream

#ifndef __LEXICALPARSER_H__
#define __LEXICALPARSER_H__

class lexicalParser {
private:
    char *fname;
    std::ifstream *is;
    char look; // lookahead character


    void Expected(std::string err);
    void fin();
    void skipWhite();
    int isWhite(char c);
    int isOp(char c);
    int isAddop(char c);
    int isAlNum(char c);
    int isDigit(char c);
    int isAlpha(char c);
    void getName(char *Name, unsigned int len);
    int getNum();
    std::string getOp();
    std::string getName();
    void getChar();

public:
    lexicalParser(char *fname);

    lexeme getNextLexeme();
};

#endif
