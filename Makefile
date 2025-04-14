all:
	g++ src/*.cpp -o main -I/usr/local/include -L/usr/local/lib -lSDL3 -lSDL3_image -Iinclude
