INCLUDE = ../include

# Compilation flags.
CFLAGS = -std=c11 -Wall -Wextra
CPPFLAGS = -I$(INCLUDE)
DEBUGFLAGS = -g

# Disable implicit rules
.SUFFIXES:

# Special rules and targets.
.PHONY: all clean help

# Rules and targets.
all: aes

aes: aes.o cipher.o key.o block.o state.o
	$(CC) $(CFLAGS) -o $@ $^

aes.o: aes.c $(INCLUDE)/define.h $(INCLUDE)/key.h $(INCLUDE)/block.h
	$(CC) $(CFLAGS) $(CPPFLAGS) $(DEBUGFLAGS) -c $<

cipher.o: cipher.c $(INCLUDE)/cipher.h $(INCLUDE)/define.h $(INCLUDE)/state.h
	$(CC) $(CFLAGS) $(CPPFLAGS) $(DEBUGFLAGS) -c $<

key.o: key.c $(INCLUDE)/key.h $(INCLUDE)/define.h
	$(CC) $(CFLAGS) $(CPPFLAGS) $(DEBUGFLAGS) -c $<

block.o: block.c $(INCLUDE)/block.h $(INCLUDE)/cipher.h $(INCLUDE)/define.h \
		$(INCLUDE)/key.h $(INCLUDE)/state.h 
	$(CC) $(CFLAGS) $(CPPFLAGS) $(DEBUGFLAGS) -c $<

state.o: state.c $(INCLUDE)/state.h $(INCLUDE)/define.h
	$(CC) $(CFLAGS) $(CPPFLAGS) $(DEBUGFLAGS) -c $<

clean:
	rm -f *.o aes *~

help:
	@echo "Usage:"
	@echo -e "  make [all]\t\tRun the target aes"
	@echo -e "  make aes\t\tBuild executable aes"
	@echo -e "  make clean\t\tRemove all files produced by the compilation"
	@echo -e "  make help\t\tDisplay this help"
