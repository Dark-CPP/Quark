# Quark
C++ Library to help create a compiler/interpreter for a programming language with ease.

## How This Works

It parses the string/file in 3 steps:

<li>Parses a string into a token array</li>
<li>Parses the token array into a node tree by applying a set of user defined rules</li>
<li>Goes thorough the node tree and executes a set of functions to use the node tree</li>

## Example

QuarkCompiler.cpp and Quark_Grammar showcase the library

## Dependencies And C++ standard

This library uses C++17 standard and depends on <a href="https://github.com/fmtlib/fmt">fmtlib</a>
