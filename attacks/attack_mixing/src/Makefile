INCLUDE = ../include
TABLES = ../../../whitebox_aes/include

# Compilation flags.
CFLAGS = -std=c11 -Wall -Wextra
CPPFLAGS = -I$(TABLES) -I$(INCLUDE)
DEBUGFLAGS = -g

# Disable implicit rules
.SUFFIXES:

# Special rules and targets.
.PHONY: all clean help

# Rules and targets.
all: collision

collision: collision.o conversion.o
	$(CC) $(CFLAGS) -o $@ $^

collision.o: collision.c $(TABLES)/tables.h $(INCLUDE)/sbox.h $(INCLUDE)/conversion.h
	$(CC) $(CFLAGS) $(CPPFLAGS) $(DEBUGFLAGS) -c $<

conversion.o: conversion.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(DEBUGFLAGS) -c $<

clean:
	rm -f *.o collision *~

help:
	@echo "Usage:"
	@echo -e "  make [all]\t\tRun target collision"
	@echo -e "  make collision\tBuild executable collision"
	@echo -e "  make clean\t\tRemove all files produced by the compilation"
	@echo -e "  make help\t\tDisplay this help"
