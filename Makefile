CXX_OPTIMAL = g++ -std=c++20 -O3 -march=native -pthread
CXX_FAST = g++ -std=c++20 -O0 -march=native -pthread
CXX_DEBUG = g++ -std=c++20 -g3 -O0 -fno-inline -march=native -pthread -Wall

all:
	@mkdir -p ./src/bin
	@$(CXX_FAST) \
	./src/main.cpp -o ./src/bin/main_not_optimal;

optimal:
	@mkdir -p ./src/bin
	@$(CXX_OPTIMAL) \
	./src/main.cpp -o ./src/bin/main;

debug:
	@mkdir -p ./src/bin
	@$(CXX_DEBUG) \
	./src/main.cpp -o ./src/bin/debug;

training:
	@mkdir -p ./src/bin
	@$(CXX_FAST) \
	./src/coaching.cpp -o ./src/bin/coaching;

see_total_memory:
	@make --no-print-directory
	@make --no-print-directory optimal
	@make --no-print-directory debug
	@make --no-print-directory training
	@du -h ./src/bin/main ./src/bin/main_not_optimal ./src/bin/debug

clean:
	@rm -rf ./src/bin FileTestForWorldParser logs
