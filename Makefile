build: build_dataframe build_serialize build_fake_network

run: run_dataframe run_serialize run_fake_network

build_dataframe:
	g++ -g -std=c++11 tests/test_dataframe.cpp -o test_dataframe.out

run_dataframe: build_dataframe
	./test_dataframe.out

build_serialize:
	g++ -g -std=c++11 tests/test_serialize.cpp -o test_serialize.out

run_serialize: build_serialize
	./test_serialize.out

build_fake_network:
	g++ -g -std=c++11 tests/test_fake_network.cpp -o test_fake_network.out

run_fake_network: build_fake_network
	./test_fake_network.out

clean:
	rm *.out

.PHONY: clean