# Compiler and flags
CC = g++
CFLAGS = -std=c++17 -Wall -Wextra

# Source files and corresponding executables
SMM_SRC = smm.cc
SCANNER_SRC = wlp4scan.cc
PARSER_SRC = wlp4parse.cc
TYPECHECKER_SRC = wlp4type.cc
CODEGENERATOR_SRC = wlp4gen.cc

SMM_EXEC = smm
SCANNER_EXEC = scan
PARSER_EXEC = parse
TYPECHECKER_EXEC = type
CODEGENERATOR_EXEC = gen

# Targets to build all executables
all: $(SMM_EXEC) $(SCANNER_EXEC) $(PARSER_EXEC) $(TYPECHECKER_EXEC) $(CODEGENERATOR_EXEC)

# Rule to build the simplified maximal munch executable
$(SMM_EXEC): $(SMM_SRC)
	$(CC) $(CFLAGS) -o $(SMM_EXEC) $(SMM_SRC)
	
# Rule to build the scanner executable
$(SCANNER_EXEC): $(SCANNER_SRC)
	$(CC) $(CFLAGS) -o $(SCANNER_EXEC) $(SCANNER_SRC)

# Rule to build the parser executable
$(PARSER_EXEC): $(PARSER_SRC)
	$(CC) $(CFLAGS) -o $(PARSER_EXEC) $(PARSER_SRC)

# Rule to build the type checker executable
$(TYPECHECKER_EXEC): $(TYPECHECKER_SRC)
	$(CC) $(CFLAGS) -o $(TYPECHECKER_EXEC) $(TYPECHECKER_SRC)

# Rule to build the code generator executable
$(CODEGENERATOR_EXEC): $(CODEGENERATOR_SRC)
	$(CC) $(CFLAGS) -o $(CODEGENERATOR_EXEC) $(CODEGENERATOR_SRC)

# Clean up the build files
clean:
	rm -f $(SMM_EXEC) $(SCANNER_EXEC) $(PARSER_EXEC) $(TYPECHECKER_EXEC) $(CODEGENERATOR_EXEC)

# PHONY target to avoid conflicts with files named 'all' or 'clean'
.PHONY: all clean
