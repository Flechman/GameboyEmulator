Nintendo Gameboy simulator

USER GUIDE:

1. Using the terminal, in the '/done' repository execute the command 'make' to compile all files.
2. Now execute the file gbsimulator and give it the ROM to execute as argument (Ex: ./gbsimulator flappyboy.gb).
3. Now you can play!

IF YOU WANT TO PLAY TETRIS: uncomment (in the Makefile) the line 'CPPFLAGS += -DNO_ROM_WRITE' l.23.

Ps: If you want more ROM just download one and put it in the Done repository and proceed as for flappy boy!

COMMANDS:

Keyboard  ==> 	Gameboy
A		A
Z		B
K		START
L	        SELECT
