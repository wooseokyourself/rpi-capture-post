all:
	g++ -std=c++11 main.cpp `pkg-config --cflags --libs opencv4` `pkg-config --cflags --libs libcurl` -o main.out 2>&1 | tee compile-output.txt

clean:
	rm -f main.out
	rm -f compile-output.txt