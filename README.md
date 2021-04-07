# CSV parser

Test assignment for google summer of code for Boost.XML.

## Installation

First, make sure `cmake` version is `17.^` and `clang` has `C++20` support on your
machine. Then execute following instructions

```bash
git clone https://github.com/Glebanister/gsoc-boost-xml
mkdir build && cd build
cmake .. && cmake --build
```

## Usage

The parser reads the XML table and rewrites it into stdout,
or tells stderr that there is a syntax error and indicates the problem.
The path to the file must be specified by the first argument.

## Example

### Valid input

```bash
$ cat input.csv
Username,Identifier,First Name,Last Name
Glebanister 1,123,Gleb,Marin
"Glebanister, 2",345,Hleb,Mapin
Some other user,identifier,Name,"last name with comma,"
$ ./cmake-build-debug/parse-csv input.csv 
'Username' 'Identifier' 'First Name' 'Last Name' 
'Glebanister 1' '123' 'Gleb' 'Marin' 
'Glebanister, 2' '345' 'Hleb' 'Mapin' 
'Some other user' 'identifier' 'Name' 'last name with comma,'
```

### Invalid input

```bash
$ cat input.csv
col1,col2,col3
value1,value2
$ ./cmake-build-debug/parse-csv input.csv 
Row length does not correspond to table width
```

```bash
$ cat input.csv
col1,col2,col3
value1,"value2,col3
$ ./cmake-build-debug/parse-csv input.csv 
Parse error: at 1:0: 'EOF' is expected here
```

## Parsing approach

Text as an array of letters is passed to parser combiners,
which find a grammatical error, or return an abstract syntactic tree.
A table is then generated from the tree.
I didn't take the easy way out by deciding to implement parser-combinators.
But this approach provided more flexibility for parsing and expansion possibilities.
