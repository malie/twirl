
CFLAGS=-O3 -g -Ipicosat-960/

twirl: twirl.o picosat-960/picosat.o
	gcc -o $@ $^

twirl.o: twirl.c

picosat-960/picosat.o: picosat-960/picosat.c
