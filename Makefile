
OBJFILE = ncc

ODIR = obj
SDIR = src

CC = gcc
CXX = g++
SIZE = size

CFLAGS = -Wall
CFLAGS += -g -std=c++11
#CFLAGS += -DDEBUG=3

UNAME := $(shell uname)

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
ifeq ($(UNAME), Linux)
	ctags -R src/*
endif
	$(SIZE) $(OBJFILE)

clean:
	rm -f $(OBJFILE)
	rm -f $(ODIR)/*
