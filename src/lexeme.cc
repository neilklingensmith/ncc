
#include "lexeme.h"


lexeme::lexeme() {

}


void lexeme::setType(int newType) {
    this->type = newType;
}

int lexeme::getType() {
    return this->type;
}

int lexeme::getSubtype() {
    return this->subtype;
}

void lexeme::setSubtype(int newSubtype) {
    this->subtype = newSubtype
}

std::string lexeme::getText() {
    return this->text;
}

void lexeme::setText(std::string newText) {
    this->text = newText;
}
