
CFLAGS=-O3 -g -Ipicosat-960/ -DTRACE

twirl: twirl.o picosat-960/picosat.o
	gcc -o $@ $^

twirl.o: twirl.c

picosat-960/picosat.o: picosat-960/picosat.c


generate:
	for i in `seq 200`; do \
	  timeout 60s ./twirl | tee log-`date +%s` | grep http; \
	done
