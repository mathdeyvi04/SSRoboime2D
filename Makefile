CXX_OPTIMAL = g++ -std=c++20 -O3 -march=native -pthread
CXX_DEBUG = g++ -std=c++20 -g3 -O0 -fno-inline -march=native -pthread -Wall

all:
	@$(CXX_OPTIMAL) \
	./src/main.cpp -o ./src/main;

debug:
	@$(CXX_DEBUG) \
	./src/main.cpp -o ./src/debug;

clean:
	@rm -rf src/debug src/main FileTestForWorldParser logs
