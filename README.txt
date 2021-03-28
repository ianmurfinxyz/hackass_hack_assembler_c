=====================================================================================================================
                         HACKASS - HACK ASSEMBLER - PROJECT 6 - NAND2TETRIS - C IMPLEMENTATION
=====================================================================================================================

                                                 [1-INTRODUCTION]

Hackass is an implementation of the Hack assembler outlined in project 6 of the online course "Nand To Tetris".

My implementation is written in pure C with a CLI modelled on the GNU C compiler gcc. My goal was to implement an
assembler that is more than a simple text processor, I wanted the assembler to have sufficient understanding of the
Hack assembly language to report useful errors to the user, in the vein of gcc.

For a brief introduction to Hack assembly, which provides context, see section 7.

                                              [2-ASSEMBLER OVERVIEW]

The assembler has the following features:

  - Two operation modes: MODE_STRIP and MODE_ASSEMBLE (see section 3).
  - Optional verbose assembly output which provides extra insight into where and why assembly may of failed.
  - Sufficient understanding of the Hack assembly language to report syntax and semantic errors to the user.
  - Simple CLI interface for ease of use.

                                                [3-OPERATION MODES]

The assembler features two operation modes, MODE_STRIP and MODE_ASSEMBLE, outlined below.

MODE_STRIP: Strips an assembly .asm file of all whitespace, comments and symbols (both labels and variables). This
  mode can be used to prepare an assembly file for loading into the CPU emulator that comes with the Nand To Tetris
  course. Note: the CPU emulator actually performs this operation itself when loading a non-stripped .asm file into
  the emulator; this assembler thus provides the capability to perform this operation externally to the emulator.

MODE_ASSEMBLE: Performs assembly on a .asm assembly file, translating the assembly commands into Hack machine
  instructions. Errors in the assembly code are reported to the user via stderr.

                                                [4-ERROR REPORTING]

Assembly code errors are reported using a similar format to the GNU c compiler gcc, that format being:

        <filename>:<line number>:error:<error_string>
          <line number> |<line string>

For example, when assembling a file "rect.asm" with an invalid mnemonic error on line 14 ("D-Y" is not a valid "comp"
mnemonic) the assembler output the following error message:

        rect.asm:14:error:invalid computation for C command of format <dest>=<comp>
          14 |M=D-Y

In total the assembler can report the following errors:

  - Invalid mnemonic used in C command, either unrecognised mnemonic or mnemonic cannot be used in that C command
      format.
  - Unrecognised instruction formats.
  - Repeat declerations of a (goto) label; labels must be unique.
  - Exceed ROM or RAM capacity.
  - Literals to large to fit in the 15-bit addresses used by the Hack computer.
  - Many general syntax errors, such as unexpected characters.

Note: the assembler works in phases and it does not progress to the next phase unless the previous phase completes
without errors. Thus fixing a set of reported errors, may result a new set of errors being reported by a latter phase.

                                                    [5-THE CLI]

The command line interface is rather standard and is easily summed up by the short help message that can be output
with the '-h' switch, printed below:

                        --------------------------------------------------------------
                        USAGE
                           hackass infile [-o outfile] [-a|-s|-h] [-v]

                        OPTIONS
                          -a    Assemble .asm infile to .hack outfile (default mode).
                          -s    Strip .asm infile of whitespace, comments and symbols.
                          -h    Print this help message.
                          -v    Print verbose assembler output to stdout.
                          -o    Specify name of outfile, default is a.out.

                        For more detailed help, please see,
                        <https://github.com/imurf/hackass-hack-assembler-c>
                        --------------------------------------------------------------

                                                   [6-KNOWN BUGS]

- some errors are reported multiple times.

                                         [7-BRIEF OVERVIEW OF HACK ASSEMBLY]

For a more complete introduction to the Hack Assembly language and the Hack computer it is designed for, please refer
to Nand To Tetris course materials.

Hack assembly is a simple language designed for the Hack computer platform that comes as part of the course. The
Hack instruction set architecture (ISA) consists of only two instructions and thus Hack assembly language also
contains only two instructions (which I refer to as commands, to distinguish them from their equivilents in the ISA):
'A' commands and 'C' commands. However C commands can take three different forms due to optional command components/fields.

The assembly commands take the following formats:

  A-Command   ->    "@<literal|symbol>"
  C-Command   ->    "<dest>=<comp>;<jump>"
              ->    "<dest>=<comp>
              ->    "<comp>;<jump>"

note: "comp" is short for computation, "dest" for destination. Further <comp>, <dest> and <jump> are placeholders
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

The binary values associated with the mnemonics correspond to the representation of the mnemonics in the ISA. The
assemblers job is of course to translate Hack assembly into Hack machine instructions defined in the ISA. The binary
format of the two ISA commands is outlined below.

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

             unused/junk bits                dest bits
                     |                           |
                   +---+                     +--------+
                   |   |                     |        |
    Binary Form:  1 1 1  a c1 c2 c3 c4 c5 c6  d1 d2 d3  j1 j2 j3
                  ^     |                   |          |        |
                  |     +-------------------+          +--------+
              op-code            |                          |
                             comp bits                   jump bits

So for example the assembly command "D=D-1;JLE" which is C command of the form <dest>=<comp>;<jump> translates to the
binary machine instruction "1110001110010110".

Finally it is of value to understand that due to the design of the Hack Computer Architecture and the Assembly
language, each C command format is restricted to a subset of mnemonics for each field, i.e not all mnemonics can
be used in all formats. In short, any C command format that contains a "jump" field cannot use a mnemonic that makes
any reference to the "A" or "M" registers in any field. Thus this makes it possible to write a syntactically valid,
semantically invalid C command. This idiosyncrasy of the Hack platform is a potential source for many subtle, hard to
find, crashes. I thus made it a design objective of my assembler to catch these errors and report them to the user.
This was the main motivation behind making the assembler more than a simple text parser.

                                                    [8-LICENSE]

              MIT License

              This project was completed by Ian Murfin as part of the Nand2Tetris Audit course
              at coursera.

              It was completed as part of my personal portfolio. Nand2tetris requires submissions
              be your own work; plagiarism is your responsibility.

              Copyright (c) 2020 Ian Murfin

              Permission is hereby granted, free of charge, to any person obtaining a copy of
              this software and associated documentation files (the "Software"), to deal in
              the Software without restriction, including without limitation the rights to
              use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
              of the Software, and to permit persons to whom the Software is furnished to do
              so, subject to the following conditions:

              The above copyright notice and this permission notice shall be included in all
              copies or substantial portions of the Software.

              THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
              IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
              FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
              COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
              IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
              WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

              End license text.

