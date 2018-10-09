SRC=src

# Disable built-in rules
.SUFFIXES:

# Special rules and targets
.PHONY: all build clean help

# Rules and targets
all: build

build:
	$(MAKE) -C $(SRC) all
	cp $(SRC)/collision .

clean:
	rm -f collision *~
	make -C $(SRC) clean

help:
	@echo "Usage:"
	@echo -e "  make [all]\t\tRun the target build"
	@echo -e "  make build\t\tBuild executables test and collision"
	@echo -e "  make clean\t\tRemove all files produced by the compilation"
	@echo -e "  make help\t\tDisplay this help"
