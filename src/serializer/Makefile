build:
	g++ -g -std=c++11 main.cpp

run: build
	./a.out

clean:
	rm a.out

test_string_array:
	cd ../test && g++ -g -std=c++11 string_array_test.cpp -o string_array_test
	cd ../test && ./string_array_test
	cd ../test && rm string_array_test

test_double_array:
	cd ../test && g++ -g -std=c++11 double_array_test.cpp -o double_array_test
	cd ../test && ./double_array_test
	cd ../test && rm double_array_test

test_message:
	cd ../test && g++ -g -std=c++11 message_test.cpp -o message_test
	cd ../test && ./message_test
	cd ../test && rm message_test

test: test_string_array test_double_array test_message

.PHONY: clean