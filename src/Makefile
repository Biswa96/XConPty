#Makefile for XConPty project

NAME := XConPty
BINDIR = ../bin
CC = gcc
CFLAGS := -Os -m64 -Wall -Wextra

#make CCOPT=-DFUN_MODE for fun mode
CCOPT := 

LIBS := -lNtdll "C:\Windows\System32\ntdll.dll"
SOURCES := $(wildcard *.c)
OBJECTS := $(SOURCES:.c=.obj)
BINS := $(patsubst %.obj,$(BINDIR)/%.obj,$(OBJECTS))

EXECUTABLE=$(BINDIR)/$(NAME)

all: $(BINDIR) $(EXECUTABLE)

$(EXECUTABLE): $(BINS)
	$(CC) $(CFLAGS) $(CCOPT) $^ $(LIBS) -o $@

$(BINS): $(BINDIR)/%.obj: %.c
	$(CC) -c $(CFLAGS) $(CCOPT) -fstack-check $< -o $(BINDIR)/$*.obj

$(BINDIR):
	@mkdir ..\bin

#END