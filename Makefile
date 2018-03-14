CC          = gcc 
CFLAGS      = -std=c99 -O3 -mssse3 -msse3 -march=native -Wstrict-aliasing -fstrict-aliasing -Wall -Wextra -pedantic -Waggressive-loop-optimizations -fno-schedule-insns -fomit-frame-pointer
INCLUDES    = -Iinclude -Iaux_include
DEFINES	    = 
SOURCES	    = $(shell find src/ -name '*.c')


build:
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $(SOURCES) aux/check.c -o check.o
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $(SOURCES) aux/bench.c aux/time.c -o time.o

time:
	./time.o

check:
	./check.o

clean:
	rm -f ./*.o
