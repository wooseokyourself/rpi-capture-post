all:
	g++ -std=c++11 main.cpp `pkg-config --cflags --libs opencv4` `pkg-config --cflags --libs libcurl` -o main.out

clean:
	rm -f main.out