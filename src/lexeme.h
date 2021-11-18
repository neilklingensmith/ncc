
#ifndef __LEXEME_H__
#define __LEXEME_H__

#include <string>

/////////////////////////////////////////////////////
// Preprocessor Definitions

// Lexeme types
#define LEXEME_TYPE_ADDOP           0x001
#define LEXEME_TYPE_MULOP           0x002
#define LEXEME_TYPE_ASSIGNMENTOP    0x003
#define LEXEME_TYPE_IDENT           0x004
#define LEXEME_TYPE_KEYWORD         0x005
#define LEXEME_TYPE_INTEGER         0x006
#define LEXEME_TYPE_SEMICOLON       0x007
#define LEXEME_TYPE_PARENTHESES     0x008
#define LEXEME_TYPE_OPENBRACE       0x009
#define LEXEME_TYPE_CLOSEBRACE      0x00A


/////////////////////////////////////////////////////
// Preprocessor Definitions

// Keyword Types
#define KEYWORD_TYPE_IF           0x101
#define KEYWORD_TYPE_INT          0x102
#define KEYWORD_TYPE_WHILE        0x103

class lexeme {

public:
    lexeme();
    void setType(int newType);
    int getType();
    void setSubtype(int newSubtype);
    int getSubtype();
    std::string getText();
    void setText(std::string newText);
    int getValue();
    void setValue(int newValue);
private:
    int type;                    // lexeme type
    int subtype;                 // keyword type
    std::string text;
    unsigned long long value;
};

#endif
