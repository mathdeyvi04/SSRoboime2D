all:
	@g++ -std=c++20 -O3 -march=native ./src/main.cpp -o ./src/main;
	@./src/main;

debug:
	@g++ -std=c++20 -g3 -O0 -fno-inline -march=native -Wall ./src/main.cpp -o ./src/debug;
	@gdb ./src/debug;
