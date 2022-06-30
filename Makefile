
OBJFILE = ncc

ODIR = obj
SDIR = src

CC = gcc
CXX = g++
SIZE = size

CFLAGS = -Wall
CFLAGS += -g
#CFLAGS += -DDEBUG=3

OBJS +=  \
	ncc.o \
	lexicalscanner.o \
	lexeme.o \
    parser.o \
    identifier.o \


OBJ = $(patsubst %,$(ODIR)/%,$(OBJS))

$(ODIR)/%.o: $(SDIR)/%.cc
	$(CXX) $(CFLAGS) -c -o $@ $^


$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $^

all: bin


bin: $(OBJ)
	$(CXX)  $(CFLAGS) obj/* -o ncc
	ctags -R src/*
	$(SIZE) $(OBJFILE)

clean:
	rm -f $(OBJFILE)
	rm -f $(ODIR)/*
