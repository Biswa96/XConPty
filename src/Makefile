#Makefile for XConPty project

NAME := XConPty
BINDIR = ../bin
CFLAGS := -std=c99 -O2 -Wall -Wextra -Wpedantic
CCOPT = -D_WIN32_WINNT=0x0A000006 -DUNICODE -D_UNICODE

LIBS := -lntdll
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
