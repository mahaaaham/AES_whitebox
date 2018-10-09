INCLUDE = ../include
TABLES = ../../../whitebox_aes/include

# Disable implicit rules
.SUFFIXES:

# Compilation flags.
CPPFLAGS = 
CFLAGS = -std=c11 -Wall -Wextra -I$(INCLUDE) -I$(TABLES)
DEBUGFLAGS = -g
LDLIBS = 
# Special rules and targets.
.PHONY: all clean help

# Rules and targets.
all: attack 

attack: attack.o signature.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

make_COL_ROW: make_COL_ROW.o signature.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

%.o: %.c 
	$(CC) $(CFLAGS) $(CPPFLAGS) $(DEBUGFLAGS) -c $<

clean:
	rm -f *.o attack make_COL_ROW *~

help:
	@echo "Usage:"
	@echo -e "  make [all]\t\tRun the target attack"
	@echo -e "  make attack\t\tBuild the executable attack"
	@echo -e "  make clean\t\tRemove all files produced by the compilation"
	@echo -e "  make help\t\tDisplay this help"
