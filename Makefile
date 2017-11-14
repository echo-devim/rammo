all:
	g++ -Wall `pkg-config --cflags gtk+-3.0` -O2 -o rammo rammo.cpp -lstdc++fs `pkg-config --libs gtk+-3.0`
