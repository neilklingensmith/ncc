

#include "identifier.h"

identifier::identifier() {
    this->type = 0;
}


identifier::identifier(unsigned int type) {
    this->type = type;
}

void identifier::setType(unsigned int newType) {
    this->type = newType;
}

unsigned int identifier::getType() {
    return this->type;
}

void identifier::setNumBytes(unsigned int newNumBytes) {
    this->nbytes = newNumBytes;
}

unsigned int identifier::getNumBytes() {
    return this->nbytes;
}

int identifier::getStackFramePosition() {
    return this->stackFramePosition;
}

void identifier::setStackFramePosition(int newStackFramePosition) {
    this->stackFramePosition = newStackFramePosition;
}


