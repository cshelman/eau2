build:
	g++ -g -std=c++11 tests/test.cpp -o test

run: build
	./test

clean:
	rm test

.PHONY: clean