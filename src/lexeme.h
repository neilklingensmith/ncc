
#ifndef __LEXEME_H__
#define __LEXEME_H__

#include <string>

/////////////////////////////////////////////////////
// Preprocessor Definitions

// Lexeme types
// These are one-hot encoded so we can logic-OR them together to search for
// multiple keyword types. We do this in the parser::declaration.
#define LEXEME_TYPE_ADDOP           0x001
#define LEXEME_TYPE_MULOP           0x002
#define LEXEME_TYPE_ASSIGNMENTOP    0x004
#define LEXEME_TYPE_IDENT           0x008
#define LEXEME_TYPE_KEYWORD         0x010
#define LEXEME_TYPE_INTEGER         0x020
#define LEXEME_TYPE_SEMICOLON       0x040
#define LEXEME_TYPE_PARENTHESES     0x080
#define LEXEME_TYPE_OPENBRACE       0x100
#define LEXEME_TYPE_CLOSEBRACE      0x200
#define LEXEME_TYPE_COMMA           0x400


/////////////////////////////////////////////////////
// Preprocessor Definitions

// Keyword Types
#define KEYWORD_TYPE_IF                  0x80000001
#define KEYWORD_TYPE_INT                 0x80000002
#define KEYWORD_TYPE_WHILE               0x80000003
#define KEYWORD_TYPE_RETURN              0x80000004



#define COMPARISON_TYPE_LESS_THAN        0x90000001
#define COMPARISON_TYPE_GREATER_THAN     0x90000002
#define COMPARISON_TYPE_LESS_OR_EQUAL    0x90000003
#define COMPARISON_TYPE_GREATER_OR_EQUAL 0x90000004
#define COMPARISON_TYPE_EQUAL            0x90000005
#define COMPARISON_TYPE_NOT_EQUAL        0x90000006

class lexeme {

public:
    lexeme();
    void setType(int newType);
    int getType();
    void setSubtype(int newSubtype);
    unsigned int getSubtype();
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
