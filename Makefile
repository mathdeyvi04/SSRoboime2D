CXX_OPTIMAL = g++ -std=c++20 -O3 -march=native
CXX_DEBUG = g++ -std=c++20 -g3 -O0 -fno-inline -march=native -Wall

all:
	@$(CXX_OPTIMAL) \
	./src/main.cpp -o ./src/main;
	@./src/main;

multithread:
	@$(CXX_OPTIMAL) \
	-pthread -DMULTITHREAD \
	-DAGENT_INFO \
	./src/main.cpp -o ./src/main;
	@./src/main;

debug:
	@$(CXX_DEBUG) \
	./src/main.cpp -o ./src/debug;
	@gdb ./src/debug;

debug_multithread:
	@$(CXX_DEBUG) \
	-pthread -DMULTITHREAD \
	./src/main.cpp -o ./src/debug;
	@gdb ./src/debug;

clean:
	@rm -rf src/debug src/main logs
