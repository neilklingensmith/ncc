

#include "identifier.h"

identifier::identifier() {
    this->type = 0;
}


identifier::identifier(unsigned int type) {
    this->arrayLength = 0; // Indicate that this identifier is not an array
    this->stackFramePosition = -1; // Flag invalid stack frame position
    this->type = type;
    this->arrayBytesPerElement = -1;
    this->global_or_local = 0;
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


void identifier::setArrayLength(unsigned int arrLen) {
    this->arrayLength = arrLen;
}

unsigned int identifier::getArrayLength() {
    return this->arrayLength;
}

unsigned int identifier::getArrayBytesPerElement() {
    return this->arrayBytesPerElement;
}

void identifier::setArrayBytesPerElement(unsigned int bytesPerElem) {
    this->arrayBytesPerElement = bytesPerElem;
}

void identifier::setGlobal() {
    this->global_or_local = IDENTIFIER_TYPE_GLOBAL;
}


void identifier::setLocal() {
    this->global_or_local = IDENTIFIER_TYPE_LOCAL;
}

int identifier::getGlobalOrLocal() {
    return this->global_or_local;
}


