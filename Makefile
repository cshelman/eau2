build: build_dataframe build_serialize build_fake_network

run: run_dataframe run_serialize run_fake_network

build_dataframe:
	g++ -g -std=c++11 tests/test_dataframe.cpp -o test_dataframe.out -lpthread

build_serialize:
	g++ -g -std=c++11 tests/test_serialize.cpp -o test_serialize.out -lpthread

build_fake_network:
	g++ -g -std=c++11 tests/test_fake_network.cpp -o test_fake_network.out -lpthread

run_dataframe: build_dataframe
	./test_dataframe.out

run_serialize: build_serialize
	./test_serialize.out

run_fake_network: build_fake_network
	./test_fake_network.out

valgrind: valgrind_dataframe valgrind_serialize valgrind_fake_network

valgrind_dataframe: build_dataframe
	valgrind --leak-check=full ./test_dataframe.out

valgrind_serialize: build_serialize
	valgrind --leak-check=full ./test_serialize.out

valgrind_fake_network: build_fake_network
	valgrind --leak-check=full ./test_fake_network.out

build_server:
	g++ -g -std=c++11 tests/test_server.cpp -o test_server.out -lpthread

run_server: build_server
	./test_server.out 127.0.0.1:8080 3

build_client:
	g++ -g -std=c++11 tests/test_client.cpp -o test_client.out -lpthread

run_client: build_client
	./test_client.out 127.0.0.11:8081 127.0.0.1:8080

run_client_2: build_client
	./test_client.out 127.0.0.12:8082 127.0.0.1:8080

run_client_3: build_client
	./test_client.out 127.0.0.13:8083 127.0.0.1:8080

build_linus:
	g++ -g -std=c++11 tests/linus.cpp -o linus.out -lpthread

run_linus: build_linus
	./linus.out 127.0.0.1:8080 3

clean:
	rm *.out

.PHONY: clean