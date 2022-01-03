


#ifndef __PARSER_H__
#define __PARSER_H__

#include "lexicalscanner.h"
#include "identifier.h"
#include "identifier.h"
#include <map>
#include <stack>

#define COMMENT_STRING ";"

class parser {
private:
    std::string *input_file_name;
    std::ostream *os;
    std::string file_name;
    void do_function_call(lexeme l, std::map<std::string, identifier*>&symbolTable, std::stack<std::string>&dataRegFreeStack, std::stack<std::string>&dataRegStatementStack, std::stack<std::string>&addrRegFreeStack, std::stack<std::string>&addrRegStatementStack);

public:
    lexicalScanner *lex;
    parser(char *ifname, char *ofname);
    void emit(std::string&);
    void emit(char*);
    void error(const char *msg);
    void function();
    void block(std::map<std::string, identifier*>&symbolTable);
    void statement(std::map<std::string, identifier*>&symbolTable);
    std::string declaration(std::map<std::string, identifier*>&symbolTable, int declaration_terminator);
    void expression(std::map<std::string, identifier*>&symbolTable, std::stack<std::string>&dataRegFreeStack, std::stack<std::string>&dataRegStatementStack, std::stack<std::string>&addrRegFreeStack, std::stack<std::string>&addrRegStatementStack);
    void term(std::map<std::string, identifier*>&symbolTable, std::stack<std::string>&dataRegFreeStack, std::stack<std::string>&dataRegStatementStack, std::stack<std::string>&addrRegFreeStack, std::stack<std::string>&addrRegStatementStack);
    void signedfactor(std::map<std::string, identifier*>&symbolTable, std::stack<std::string>&dataRegFreeStack, std::stack<std::string>&dataRegStatementStack, std::stack<std::string>&addrRegFreeStack, std::stack<std::string>&addrRegStatementStack);
    void factor(std::map<std::string, identifier*>&symbolTable, std::stack<std::string>&dataRegFreeStack, std::stack<std::string>&dataRegStatementStack, std::stack<std::string>&addrRegFreeStack, std::stack<std::string>&addrRegStatementStack);

};
#endif
