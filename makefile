CFLAGS=-std=cll -g -static

main: main.cpp
9cc: main.cpp

test: 9cc
	./test.sh

clean:
	rm -f main *.o *~ tmp*

.PHONY: test clean