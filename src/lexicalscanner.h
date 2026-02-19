


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
    int linePrintingPaused = 0; // Indicates whether line printing is paused
    std::string paused_output; // Output of lexical scanner while paused

    void Expected(std::string err);
    void fin();
    void skipWhite();
    void printNextLine();
    int isWhite(char c);
    int isOp(char c);
    int isAddop(char c);
    int isAlNum(char c);
    int isDigit(char c);
    int isHexDigit(char c);
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

    // pauseLinePrinting() causes the lexicalScanner to pause printing commented
    // lines of C to its output. Used when generating stack frames. When
    // resumeLinePrinting() is called, the lexicalScanner will immediately dump
    // all lines of C that it would have printed while paused.
    void pauseLinePrinting();
    void resumeLinePrinting();
};

#endif
