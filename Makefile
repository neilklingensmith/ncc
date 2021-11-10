
OBJFILE = ncc

ODIR = obj
SDIR = src

CC = gcc
CXX = g++
SIZE = size

CFLAGS = -Wall
CFLAGS += -g

OBJS +=  \
	ncc.o \
	lexicalparser.o \
	lexeme.o \


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
