


#include "lexeme.h"
#include <iostream>     // std::cin, std::cout
#include <fstream>      // std::ifstream

#ifndef __LEXICALSCANNER_H__
#define __LEXICALSCANNER_H__

class lexicalScanner {
private:
    char *fname;
    std::ifstream *is;
    int look; // lookahead character
    unsigned int currLineNumber;
    unsigned int currColumnNumber;
    lexeme currLexeme;


    void Expected(std::string err);
    void fin();
    void skipWhite();
    int isWhite(char c);
    int isOp(char c);
    int isAddop(char c);
    int isAlNum(char c);
    int isDigit(char c);
    int isAlpha(char c);
    int isRelOp(char c);
    void getName(char *Name, unsigned int len);
    int getNum();
    std::string getRelOp();
    std::string getOp();
    std::string getName();
    void getChar();

public:
    lexicalScanner(char *fname);

    lexeme getNextLexeme();
    lexeme peekLexeme();
    unsigned int getCurrLineNumber();
    unsigned int getCurrColumnNumber();
};

#endif
