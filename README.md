# Blur
This project is an attempt to implement a copy-and-patch compiler. 
This is a compilation technique described by Haoran Xu and Fredrik Kjolstad in their [Copy-and-Patch Compilation](https://arxiv.org/abs/2011.13127) paper, 
and further explored in the wonderful [Deegen JIT VM generator](https://arxiv.org/abs/2411.11469) paper.

The motiviation behind this work is to explore their technique and implement a small copy-and-patch compiler in C using [tcc](https://bellard.org/tcc/) as a backend.

## Copy-and-Patch
Copy-and-Patch Compilation is a compiler technique that precompiles generated source code to *binary stencils*. The stencils implement the functionality of a program,
but with *holes* that can be filled in with a programs constant values. 
For instance, instead of traditionally compiling an addition operation, we simply copy *addition stencil*, and fill in the arguments.
This is done by pattern matching on the abstract syntax tree of the code, which makes it very efficient.

Another benefit of Copy-and-Patch is that you get portability for free. You can compile the *binary stencils* for any architecture your backend compiler supports. 
This includes instruction set architechtures (ISAs) and calling conventions. 

## Setup
To build `blur`, execute the following:
```
git submodule update --init
make run
```
This will build the `tcc` compiler from source, and use it to generate stencils and run an example program.


## This work
So far, a proof of concept stencil generator for addition is implemented, as well as a patching example for this stencil. 

This proof of concept compiles a simple adder function. It contains two sentinel values that are placeholders for patching later. These are `0xfffffff0` and `0xfffffff1`.
```
int add() {
    int a = 0xfffffff0;
    int b = 0xfffffff1;
    int c = a + b;
    return c;
}
```
When compiled, we get the following machine code.
```
0000000  55  48  89  e5  48  81  ec  10  00  00  00  b8 [f0  ff  ff  ff]
0000010  89  45  fc  b8 [f1  ff  ff  ff] 89  45  f8  8b  45  fc  8b  4d
0000020  f8  01  c8  89  45  f4  8b  45  f4  c9  c3  62  6c  75  72  2b
0000030  00  00  00  0c  00  00  00  14  00  00  00
```
Here, we can see two blocks of interest, namely `[f0  ff  ff  ff]` at the end of line one and `[f1  ff  ff  ff]` in line two (marked for clarity). 
These are the values we want to note as *holes*. A simply `memcpy` operation is used to find these, and mark their index in the code.
Then, during *Copy-and-Patch*, we simply compy the entire code block, and replace these *hole* values with the numbers we want to add!


If say want to add the integers `1` and `2`, we replace these *holes* with the corresponding integer representation:
```
0000000  55  48  89  e5  48  81  ec  10  00  00  00  b8 [01  00  00  00]
0000010  89  45  fc  b8 [02  00  00  00] 89  45  f8  8b  45  fc  8b  4d
0000020  f8  01  c8  89  45  f4  8b  45  f4  c9  c3  62  6c  75  72  2b
0000030  00  00  00  0c  00  00  00  14  00  00  00
```

Which when executed returns the number `5`. Hurra!

## Further work
A lot is left to do here!
