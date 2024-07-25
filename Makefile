all:
	cc src/*.c -Isrc/include -Iraylib/include -Lraylib/lib/ -l:libraylib.a -lm -o bin/game
