GSL = ../../../gsl/include
INCLUDE = ../include
TABLES = ../../whitebox_aes/include

# Disable implicit rules
.SUFFIXES:

# Compilation flags.
CPPFLAGS = -I$(INCLUDE) -I$(GSL)
CFLAGS = -std=c11 -Wall -Wextra
DEBUGFLAGS = -g
LDFLAGS = -L../../../gsl/lib
LDLIBS = -lgsl -lgslcblas -lm

# Special rules and targets.
.PHONY: all clean help

# Rules and targets.
all: generate_tables 

generate_tables: generate_tables.o inner_function.o key.o matrix.o table_writing.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@
	mv generate_tables ..

generate_tables.o: generate_tables.c $(INCLUDE)/inner_function.h\
		$(INCLUDE)/key.h $(INCLUDE)/matrix.h \
		$(INCLUDE)/sbox.h $(INCLUDE)/table_writing.h $(INCLUDE)/test.h
	$(CC) $(CFLAGS) $(CPPFLAGS) $(DEBUGFLAGS) -c $<

inner_function.o: inner_function.c $(INCLUDE)/inner_function.h\
		$(INCLUDE)/key.h $(INCLUDE)/matrix.h \
		$(INCLUDE)/sbox.h $(INCLUDE)/table_writing.h
	$(CC) $(CFLAGS) $(CPPFLAGS) $(DEBUGFLAGS) -c $<

key.o: key.c $(INCLUDE)/key.h $(INCLUDE)/sbox.h
	$(CC) $(CFLAGS) $(CPPFLAGS) $(DEBUGFLAGS) -c $<

table_writing.o: table_writing.c $(INCLUDE)/table_writing.h \
		$(INCLUDE)/inner_function.h $(INCLUDE)/key.h $(INCLUDE)/sbox.h 
	$(CC) $(CFLAGS) $(CPPFLAGS) $(DEBUGFLAGS) -c $<

%.o: %.c $(INCLUDE)/%.h
	$(CC) $(CFLAGS) $(CPPFLAGS) $(DEBUGFLAGS) -c $<

clean:
	rm -f *.o ../generate_tables $(TABLES)/tables.h *~

help:
	@echo "Usage:"
	@echo -e "  make [all]\t\tRun the target generate_tables"
	@echo -e "  make generate_tables\tBuild executable generate_tables in ../"
	@echo -e "  make clean\t\tRemove all files produced by the compilation"
	@echo -e "  make help\t\tDisplay this help"
