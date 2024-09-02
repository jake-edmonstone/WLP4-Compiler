# WLP4 Compiler

This project is a compiler from WLP4 (a small subset of C) to MIPS assembly language. The specification for WLP4 can be found online. The compiler consists of four main components: a scanner, parser, type checker, and code generator. Each component is implemented as a separate C++ program.

## Components

- **Scanner (`smm`** and **`scan`)**: Tokenizes the input source code.
- **Parser (`parse`)**: Analyzes the tokenized input and constructs a parse tree.
- **Type Checker (`type`)**: Validates the syntax tree against type rules.
- **Code Generator (`gen`)**: Generates target assembly from the validated syntax tree.

## Usage

To compile the project, use the following commands:

```bash
make
```
This will create the following executables:

- smm
- scan
- parse
- type
- gen

Each executable can be run individually, processing the output from the previous stage:
```bash
cat test.wlp4 | ./smm | ./scan | ./parse | ./type | ./gen
```
