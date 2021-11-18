


#ifndef __PARSER_H__
#define __PARSER_H__

#include "lexicalscanner.h"

class parser {
private:
    lexicalScanner *lex;
    std::string file_name;

public:
    parser(char *fname);
    void error(const char *msg);
    void block();
    void statement();
    void expression();
    void term();
    void signedfactor();
    void factor();

};
#endif
