all:
	@g++ -std=c++20 -O3 -march=native ./src/main.cpp -o ./src/main;
	@./src/main;

multithread:
	@g++ -std=c++20 -O3 -pthread -DMULTITHREAD -march=native ./src/main.cpp -o ./src/main;
	@./src/main;

debug:
	@g++ -std=c++20 -g3 -O0 -fno-inline -march=native -Wall ./src/main.cpp -o ./src/debug;
	@gdb ./src/debug;

debug_multithread:
	@g++ -std=c++20 -g3 -O0 -pthread -DMULTITHREAD -fno-inline -march=native -Wall ./src/main.cpp -o ./src/debug;
	@gdb ./src/debug;

clean:
	@rm -rf src/debug src/main logs
