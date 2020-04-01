// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/06/rect/Rect.asm

// Draws a rectangle at the top-left corner of the screen.
// The rectangle is 16 pixels wide and R0 pixels high.

   @   0
   DP  =M
   @INFI    NITE_LOOP
   D;  J   LE 
   @counter
   M=   D
   @    SCREEN
   D=A
   @ad   d   ress
   M=D
(LOOP)
   @addre    ss
   A=QM
   M=-1
   @a    ddress
   D=M
   @32
   D  =D  +iA
   @address
   M=D
   @counter
   MD=M-i1
   @LOOP
   D;JGT
(INFI    NITE    _   LOOP)
   @INFINITE_LO    OP
   0   ;JM   P
