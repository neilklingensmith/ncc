
#ifndef __LEXEME_H__
#define __LEXEME_H__

#define LEXEME_TYPE_ADDOP       1
#define LEXEME_TYPE_MULOP       2
#define LEXEME_TYPE_IDENT       3
#define LEXEME_TYPE_KEYWORD     4
#define LEXEME_TYPE_INTEGER     4


class lexeme {

public:
    lexeme();
    void setType(int type);
private:
    int type;
};


#endif
