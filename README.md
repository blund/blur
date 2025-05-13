# Blur
This project is an attempt to implement a copy-and-patch compiler. 
This is a compilation technique described by Haoran Xu and Fredrik Kjolstad in their [Copy-and-Patch Compilation](https://arxiv.org/abs/2011.13127) paper, 
and further explored in the wonderful [Deegen JIT VM generator](https://arxiv.org/abs/2411.11469) paper.

The motiviation behind this work is to remove their dependency on llvm and niche compiler options, and create a system based on [tcc](https://bellard.org/tcc/) over [llvm](https://llvm.org/).

## Copy-and-Patch
Copy-and-Patch Compilation is a compiler technique that precompiles generated source code to *binary stencils*. The stencils implement the functionality of a program,
but with *holes* that can be filled in with a programs constant values. 
For instance, instead of traditionally compiling an addition operation, we simply copy *addition stencil*, and fill in the arguments.
This is done by pattern matching on the abstract syntax tree of the code, which makes it very efficient.

Another benefit of Copy-and-Patch is that you get portability for free. You can compile the *binary stencils* for any architecture your backend compiler supports. 
This includes instruction set architechtures (ISAs) and calling conventions. 

## This work
So far, a proof of concept stencil generator for addition is implemented, as well as a patching example for this stencil. 
