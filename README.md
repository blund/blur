# Blur
This project is an attempt to implement a copy-and-patch compiler. 
This is a compilation technique described by Haoran Xu and Fredrik Kjolstad in their [Copy-and-Patch Compilation](https://arxiv.org/abs/2011.13127) paper, 
and further explored in the wonderful [Deegen JIT VM generator](https://arxiv.org/abs/2411.11469) paper.

The motiviation behind this work is to explore their technique and implement a small copy-and-patch compiler in C.

## Copy-and-Patch
Copy-and-Patch Compilation is a compiler technique that precompiles generated source code to *binary stencils*. The stencils implement the functionality of a program,
but with *holes* that can be filled in with a programs constant values. 
For instance, instead of traditionally compiling an addition operation, we simply copy *addition stencil*, and fill in the arguments.
This is done by pattern matching on the abstract syntax tree of the code, which makes it very efficient.

Another benefit of Copy-and-Patch is that you get portability for free. You can compile the *binary stencils* for any architecture your backend compiler supports. 
This includes instruction set architechtures (ISAs) and calling conventions. 

## Running
To try out the system, simply type
```
make
```

This should generate stencils, compile them to machine code and execute the `blur` executable. This executable compiles a simple program using copy-and-patch at runtime, and executes it from memory.

## This work
This project is an attempt to implement a Copy-and-Patch compiler. The general structure of the system is shown in the graph below:
```                 
           ┌────────────┐    ┌────────────────┐    ┌────────────────────────┐
           │ Generate   │    │ Create machine │    │ Copy-and-Patch         │                        
           │ C code for │───►│ code stencils  │───►│ stencils to executable │                        
           │ stencils   │    │ with holes     │    │ binary at runtime      │                        
           └────────────┘    └────────────────┘    └────────────────────────┘                        

```

This project implements the entire pipeline from stencil generation to runtime compilation. 
Currently, only an if-statement and addition is implemented.

### Stencils, Holes, Patching
This is the implementation for one of the stencils, the `add_const` stencil:
```
void add_const(int lhs) {
  int result = lhs + 0x3e7a91bc;
  ((void (*)(uintptr_t, int))(0xe2d9c7b1843a56f0))(result);
}
```
The C code is generated in the `gen.c` file, from an AST describing its structure.
This function performs an addition, and then calls a function with the result of that addition. This style is known as *continuation-passing style*, or CPS. 

There are two sentinel values in this code, `0x3e7a91bc` and `0xe2d9c7b1843a56f0`. These are arbitrary values set to denote the *holes*, or values in the machine code we want to replace during the copy-and-patch compilation.
When *cutting the stencil*, a program inspects the machine code for this functions, and identifies where these values are. They are marked respectively as a 32-bit hole and a 64-bit hole.


When compiled, we get the following machine code.
```
0000000:       81 c6 [ bc 91 7a 3e ]              add    $0x3e7a91bc,%esi
0000006:       48 b8 [ f0 56 3a 84 b1 c7 d9 e2 ]  movabs $0xe2d9c7b1843a56f0,%rax    
00000a0:       ff e0                              jmp    *%rax
```
Here, I have marked out two blocks of interests with square brackets. These are the same values as our sentinels from before, only as they are stored in memory in little endian notation.
During copy-and-patch compilation, we can replace these values with any value we want. 
To give an example, say we want to replace the first stentinel with the value `0x02` and the second with the address of a function that prints the result, `0x000055aabbccdde0`. This procedure is as simple as writing new bytes to their locations in memory:

```
0000000:       81 c6 [ 02 00 00 00 ]              add    $0x3e7a91bc,%esi
0000006:       48 b8 [ e0 dd cc bb aa 55 00 00 ]  movabs $0xe2d9c7b1843a56f0,%rax    
00000a0:       ff e0                              jmp    *%rax
```
If we now call the function, `add_const(5)`, we now get the result `7`!

By combining stencils in continuation-passing style, we are able to craft efficient machine code for programs from these granular stenils. This is the beautfy of copy-and-patch compilation!

