#ifndef __IDENTIFIER_H__
#define __IDENTIFIER_H__


#define IDENTIFIER_TYPE_INTEGER   0x301
#define IDENTIFIER_TYPE_POINTER   0x302
#define IDENTIFIER_TYPE_CHAR      0x303

class identifier {
private:
    unsigned int arrayLength;
    unsigned int type;
    unsigned int nbytes;
    unsigned int arrayBytesPerElement;
    int stackFramePosition; // Relative to the frame pointer
public:
    identifier(unsigned int type);
    identifier();
    void setType(unsigned int newType);
    unsigned int getType();
    void setNumBytes(unsigned int newNumBytes);
    unsigned int getNumBytes();
    void setStackFramePosition(int newPosition);
    int getStackFramePosition();
    void setArrayLength(unsigned int arrLen);
    unsigned int getArrayLength();
    unsigned int getArrayBytesPerElement();
    void setArrayBytesPerElement(unsigned int bytesPerElem);
};
#endif
