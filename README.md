# WLP4 Compiler

This project is a compiler from WLP4 (a small subset of C) to MIPS assembly language. A quick overview of WLP4 can be found [here](https://student.cs.uwaterloo.ca/~cs241/wlp4/WLP4tutorial.html) or the full specification, [here](https://student.cs.uwaterloo.ca/~cs241/wlp4/). The compiler consists of four main components: a scanner, parser, type checker, and code generator. Each component is implemented as a separate C++ program.

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
cat example.wlp4 | ./smm | ./scan | ./parse | ./type | ./gen
```
