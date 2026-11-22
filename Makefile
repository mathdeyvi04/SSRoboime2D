CXX_OPTIMAL = g++ -std=c++20 -O3 -march=native -pthread
CXX_FAST = g++ -std=c++20 -O0 -march=native -pthread
CXX_DEBUG = g++ -std=c++20 -g3 -O0 -fno-inline -march=native -pthread -Wall
PARSER_TEST_FLAGS = -std=c++20 -O0 -g -D_GLIBCXX_ASSERTIONS -Wall -Wextra -Wpedantic
PARSER_SANITIZER_FLAGS = -fsanitize=address,undefined -fno-omit-frame-pointer

.PHONY: test-parser test-parser-sanitize

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

test-parser:
	@mkdir -p ./src/bin
	@g++ $(PARSER_TEST_FLAGS) tests/world_parser_test.cpp -o ./src/bin/world_parser_test
	@./src/bin/world_parser_test

test-parser-sanitize:
	@mkdir -p ./src/bin
	@g++ $(PARSER_TEST_FLAGS) $(PARSER_SANITIZER_FLAGS) tests/world_parser_test.cpp -o ./src/bin/world_parser_test_sanitize
	@ASAN_OPTIONS=detect_leaks=0 ./src/bin/world_parser_test_sanitize
	@g++ $(PARSER_TEST_FLAGS) $(PARSER_SANITIZER_FLAGS) src/environment/FileTestForWorldParser.cpp -o ./src/bin/parser_corpus_sanitize
	@ASAN_OPTIONS=detect_leaks=0 ./src/bin/parser_corpus_sanitize 4
	@ASAN_OPTIONS=detect_leaks=0 ./src/bin/parser_corpus_sanitize 5

see_total_memory:
	@make --no-print-directory
	@make --no-print-directory optimal
	@make --no-print-directory debug
	@make --no-print-directory training
	@du -h ./src/bin/main ./src/bin/main_not_optimal ./src/bin/debug

clean:
	@rm -rf ./src/bin FileTestForWorldParser logs
