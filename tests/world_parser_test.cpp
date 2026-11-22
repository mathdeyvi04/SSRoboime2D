#include "../src/environment/Environment.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

namespace {
void require(bool condition, std::string_view description) {
    if(!condition) {
        std::cerr << "FAILED: " << description << '\n';
        std::exit(1);
    }
}

void valid_see() {
    Environment env {};
    env.m_unum = 1;
    require(env.m_wp.update_from_server("(see 12 ((b) 5 30))", env), "valid see accepted");
    require(env.m_number_visibles == 1, "valid ball is visible");
    require(env.m_visibles_index[0] == 0, "ball index");
    require(env.m_points_on_the_field[0].attrs[0] == 5, "ball distance");
    require(env.m_points_on_the_field[0].attrs[1] == 30, "ball direction");
    require(Environment::CYCLE_SEE == 12, "valid cycle");
}

void invalid_see_keeps_previous_observation(std::string_view message) {
    Environment env {};
    env.m_unum = 1;
    require(env.m_wp.update_from_server("(see 20 ((b) 5 30))", env), "setup see accepted");
    require(!env.m_wp.update_from_server(message, env), "invalid see rejected");
    require(env.m_number_visibles == 1, "invalid message preserves visible count");
    require(env.m_visibles_index[0] == 0, "invalid message preserves ball index");
    require(env.m_points_on_the_field[0].attrs[0] == 5, "invalid message preserves distance");
    require(env.m_points_on_the_field[0].attrs[1] == 30, "invalid message preserves direction");
    require(Environment::CYCLE_SEE == 20, "invalid message preserves cycle");
}
}

int main() {
    valid_see();
    invalid_see_keeps_previous_observation("(see 21 ((b) 1 2 3 4 5 6 7))");
    invalid_see_keeps_previous_observation("(see 21 ((b) 1 2");
    invalid_see_keeps_previous_observation("(see 21 ((b) nope 2))");

    std::string too_many_objects = "(see 21";
    for(int i = 0; i < 83; ++i) {
        too_many_objects += " ((b) 1 2)";
    }
    too_many_objects += ')';
    invalid_see_keeps_previous_observation(too_many_objects);

    std::cout << "world_parser_test: OK\n";
}
