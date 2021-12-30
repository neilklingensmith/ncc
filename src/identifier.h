#ifndef __IDENTIFIER_H__
#define __IDENTIFIER_H__


#define IDENTIFIER_TYPE_INTEGER   0x301

class identifier {
private:
    unsigned int type;
    unsigned int nbytes;
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
};
#endif
