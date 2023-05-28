# Changes Made to the Baseline
- Added support for different integer types: short, int, long
- Added if statements, evaluates to true of the expression is a 0
- Added functions, function definitions, return types and calls are supported. The code will start execution with the main function.

# CSF363 Baseline Language

Welcome to CS F363! This is the baseline language project that will be used for all further labs/assignments  in this course.

## The Base Language

As it is a baseline, the Base language only has arithmetic, variables and a hardcoded `dbg`(debug) statement. This is how a program in the Base language currently looks like:

```
let abc = 123 * 20 + (22 / 2) - 3;
dbg abc + abc;
```

As the course progress, you will be required to make changes to the Base language to fit certain specifications.

## Installing requirements

These assignments require a *NIX environment. You can run these commands to install all the software needed to compile this project properly:

For Linux

```bash
# the tool used to build this project
sudo apt-get install make
# the tool used for lexing/scanning
sudo apt-get install flex
# the tool used for parsing
sudo apt-get install bison
# the LLVM toolchain
sudo apt-get install clang llvm
```
For MacOS (Unix)

```bash
# install Homebrew if you haven't already
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
# the tool used to build this project
make install
# the tool used for lexing/scanning
brew install flex
# the tool used for parsing
brew install bison
export PATH="$(brew --prefix bison)/bin:$PATH"
# the LLVM toolchain
echo 'export PATH="/usr/local/opt/llvm/bin:$PATH"' >> ~/.bash_profile
source ~/.bash_profile
```

## Running this project

To build this compiler simply run `make compiler`. You can find the executable called `bin/base` folder. This is your compiler.

To run this compiler simply run `make program`. This would use the [`test.be`](test.be) file and generate an executable of that program called `bin/test`. This binary is dependent on the [`runtime/runtime_lib.cc`](runtime/runtime_lib.cc) file so make sure it exists.

## Directory structure

```
CSF363-baseline
├── include
│   ├── ast.hh
│   ├── llvmcodegen.hh
│   ├── parser_util.hh
│   └── symbol.hh
├── Makefile
├── README.md
├── runtime
│   └── runtime_lib.cc
├── src
│   ├── ast.cc
│   ├── lexer.lex
│   ├── llvmcodegen.cc
│   ├── main.cc
│   ├── parser.yy
│   └── symbol.cc
└── test.be
│  
└── docs
    ├── lexer.md
    ├── parser.md
    └── llvm.md
```

- The bin folder contains the executable after running `make`
- The [`include`](include) folder contains all header files. All subsequent header files you write must be saved in this folder.
    - [`include/ast.hh`](include/ast.hh) contains the definition of the the AST of this language. We have defined one parent class `Node` as a pure abstract class (similiar to an interface in Java). All types of nodes inherit from this one `Node` class. By following the definition of the various node types, the tree structure of these ndoes becomes clearer.
    - [`include/llvmcodegen.hh`](include/llvmcodegen.hh) contains the definition of the compiler which emits LLVM-IR code.
    - [`include/parser_util.hh`](include/parser_util.hh) contains the definition of a struct to help bison parse. You will learn more about how bison works as the course progresses.
    - [`symbol.hh`](symbol.hh) contains the definition of rudimentary symbol table. As you add language constructs like scoping and functions, the structure of the symbol table will become more complicated. For now this jst keeps track of the variables that have been declared. The parser uses it ensure variables are not redeclared, and undeclared variables are not used.
- The [`src`](src) folder contains the implementation files for the header files. All subsequent implementation files you write mus be saved in this folder.
    - [`src/lexer.lex`](src/lexer.lex) contains the specification of the scanner. Each token's regex has a subsequent action. In this file, the action is to return the corresponding token that is defined in `src/parser.yy` at lines 28-31. This file uses the flex lexer generator tool.
    - [`src/parser.yy`](src/parser.yy) contains the specification of the parser and the overall grammar of the language. This parser builds an AST as defined in [`include/ast.hh`](include/ast.hh). This files uses the bison parser generator tool.
    - [`src/main.cc`](src/main.cc) is the main driver file.

- The [`docs`](docs) contains the in depth explaination of lexer , parser and llvm codegen files .  

> `include/parser.hh` is header file that you may think is missing, but it is actually generated by bison with `src/bison.yy` as input. You should see it in [`include`](include) after running `make` once.


## Additional Reading

- [Writing Your Own Toy Compiler Using Flex, Bison and LLVM](https://gnuu.org/2009/09/18/writing-your-own-toy-compiler/)
- [Flex & Bison](https://web.iitd.ac.in/~sumeet/flex__bison.pdf)
- [The Official LLVM Tutorial](https://llvm.org/docs/tutorial/index.html)
- [Crafting Interpreters](http://craftinginterpreters.com/contents.html)
