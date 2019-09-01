#Makefile for XConPty project

NAME := XConPty
BINDIR = ../bin
CFLAGS := -std=c89 -O2 -m64 -Wall -Wextra

# `make CCOPT=-DFUN_MODE` for fun mode
CCOPT := 

LIBS := -lAdvapi32 -lNtdll
SOURCES := $(wildcard *.c)
OBJECTS := $(SOURCES:.c=.obj)
BINS := $(patsubst %.obj,$(BINDIR)/%.obj,$(OBJECTS))

EXECUTABLE=$(BINDIR)/$(NAME)

all: $(BINDIR) $(EXECUTABLE)

$(EXECUTABLE): $(BINS)
	$(CC) $(CFLAGS) $(CCOPT) $^ $(LIBS) -o $@

$(BINS): $(BINDIR)/%.obj: %.c
	$(CC) -c $(CFLAGS) $(CCOPT) $< -o $(BINDIR)/$*.obj

$(BINDIR):
	@mkdir ..\bin

#END