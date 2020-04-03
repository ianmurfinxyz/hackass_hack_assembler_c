=======================================================================================================================
                         HACK ASSEMBLER - PROJECT 6 - NAND2TETRIS - C IMPLEMENTATION
=======================================================================================================================

                                                 [INTRODUCTION]

This project is an implementation of the Hack assembler outlined in project 6 of the online course "Nand To Tetris".

My implementation is written in pure C with a CLI modelled on the GNU C compiler gcc. My goal was to implement an
assembler that is more than a simple text processor, I wanted the assembler to have sufficient understanding of the
Hack assembly language to report useful errors to the user, in the vein of gcc.

                                         [BRIEF OVERVIEW OF HACK ASSEMBLY]

For a more complete introduction to the Hack Assembly language and the Hack computer it is designed for, please refer
to Nand To Tetris course materials.

Hack assembly is a simple language designed for the Hack computer platform that comes as part of the course. The
Hack instruction set architecture (ISA) consists of only two instructions and thus Hack assembly language also
contains only two instructions (which I commonly refer to as commands): 'A' commands and 'C' commands. However C
commands can take three different forms due to optional command components/fields.

The assembly commands take the following formats:

  A-Command   ->    "@<literal|symbol>"
  C-Command   ->    "<dest>=<comp>;<jump>"
              ->    "<dest>=<comp>
              ->    "<comp>;<jump>"

note: "comp" is short for computation, "dest" for destination. Further <comp>, <dest> and <jump> are place holders
for assembly mnemonics.

The function of the two commands is as follows. A-commands load a value into the address register of the Hack
computer. C-commands instruct the computer to perform one of a fixed, limited set, of computations and (a) assign the
result to a specified register, (b) use the result as a jump condition to jump to another instruction, or (c) both.

Hack assembly language also contains one psuedo command called an 'L' command that does not translate into a Hack
machine instruction. Its purpose is equivilent to a goto label in C, it simply marks a location in the code to jump
to. The effect of an L command is to create a new symbol in the assemblers symbol table which maps to an address in
ROM where the instruction to jump to is stored.

In terms of vocabulary, Hack assembly defines a small set of mnemonics for the "dest", "comp" and "jump" fields of
the C commands, and a small set of predefined symbols to be used in the A commands. These are outlined in the below
tables.
                                                      +---------------------------------------------+
       +--------------------+  +--------------------+ |comp     c1   c2   c3   c4   c5   c6    comp |
       |dest    d1   d2   d3|  |jump    j1   j2   j3| |(a=0)                                   (a=1)|
       |--------------------|  |--------------------| |---------------------------------------------|
       |null    0    0    0 |  |null    0    0    0 | |0        1    0    1    0    1    0          |
       |M       0    0    1 |  |JGT     0    0    1 | |1        1    1    1    1    1    1          |
       |D       0    1    0 |  |JEQ     0    1    0 | |-1       1    1    1    0    1    0          |
       |MD      0    1    1 |  |JGE     0    1    1 | |D        0    0    1    1    0    1          |
       |A       1    0    0 |  |JLT     1    0    0 | |A        1    1    0    0    0    0      M   |
       |AM      1    0    1 |  |JNE     1    0    1 | |!D       0    0    1    1    0    1          |
       |AD      1    1    0 |  |JLE     1    1    0 | |!A       1    1    0    0    0    1      -M  |
       |AMD     1    1    1 |  |JMP     1    1    1 | |D+1      0    1    1    1    1    1          |
       +--------------------+  +--------------------+ |A+1      1    1    0    1    1    1      M+1 |
            +---------------------+                   |D-1      0    0    1    1    1    0          |
            |symbol    RAM address|                   |A-1      1    1    0    0    1    0      M-1 |
            |---------------------|                   |D+A      0    0    0    0    1    0      D+M |
            |SP        0          |                   |D-A      0    1    0    0    1    1      D-M |
            |LCL       1          |                   |A-D      0    0    0    1    1    1      M-D |
            |ARG       2          |                   |D&A      0    0    0    0    0    0      D&M |
            |THIS      3          |                   |D|A      0    1    0    1    0    1      D|M |
            |THAT      4          |                   +---------------------------------------------+
            |R0-R15    0-15       |                     ^
            |SCREEN    16384      |                     |
            |KBD       24576      |              the mnemonics
            +---------------------+

note: tables sourced from Chapter 6: Elements of Computing Systems.

The binary values associated with the mnemonics correspond to the representation of the mneomics in the ISA. The
assemblers job is of course to translate Hack assembly into Hack machine instructions defined in the ISA as
outlined below.

A-instruction (ISA):

                op-code
                   |
                   V
     Binary Form:  0 v v v  v v v v  v v v v  v v v v           note: 16-bit instructions
                    |                                |
                    +--------------------------------+
                                |
                          value of symbol|literal from A-command @<value|symbol>


C-instruction (ISA):

                                                dest bits
               op-code                           |
                  |                          +--------+
                  V                          |        |
    Binary Form:  1 1 1  a c1 c2 c3 c4 c5 c6  d1 d2 d3  j1 j2 j3
                        |                   |          |        |
                        +-------------------+          +--------+
                                 |                          |
                             comp bits                   jump bits

Finally it is of value to understand that due to the design of the Hack Computer Architecture and the Assembly
language, each C command format is restricted to a subset of mnemonics for each field, i.e not all mnemonics can
be used in all formats. In short, any C command format that contains a "jump" field cannot use a mnemonic that makes
any reference to the "A" or "M" registers in any field. Thus this makes it possible to write a syntactically valid,
semantically invalid C command. This idiosyncrasy of the Hack platform is a potential source for many subtle, hard to
find, crashes. I thus made it a design objective of my assembler to catch these errors and report them to the user.
This was the main motivation behind making the assembler more than a simple text parser.














