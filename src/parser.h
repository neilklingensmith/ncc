


#ifndef __PARSER_H__
#define __PARSER_H__

#include "lexicalscanner.h"
#include "identifier.h"
#include "identifier.h"
#include <map>




class parser {
private:
    std::ostream *os;
    lexicalScanner *lex;
    std::string file_name;

public:
    parser(char *ifname, char *ofname);
    void emit(std::string&);
    void error(const char *msg);
    void block();
    void statement(std::map<std::string, identifier*>&symbolTable);
    void declaration(std::map<std::string, identifier*>&symbolTable);
    void expression();
    void term();
    void signedfactor();
    void factor();

};
#endif
