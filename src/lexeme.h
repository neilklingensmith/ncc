
#ifndef __LEXEME_H__
#define __LEXEME_H__

#define LEXEME_TYPE_ADDOP         1
#define LEXEME_TYPE_MULOP         2
#define LEXEME_TYPE_ASSIGNMENTOP  3
#define LEXEME_TYPE_IDENT         4
#define LEXEME_TYPE_KEYWORD       5
#define LEXEME_TYPE_INTEGER       6
#define LEXEME_TYPE_SEMICOLON     7
#define LEXEME_TYPE_PARENTHESES   8
#define LEXEME_TYPE_OPENBRACE     9
#define LEXEME_TYPE_CLOSEBRACE    10


class lexeme {

public:
    lexeme();
    void setType(int type);
    int getType();
private:
    int type;
};


#endif
