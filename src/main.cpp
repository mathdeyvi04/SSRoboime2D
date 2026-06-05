#include "./agent/BasicAgent.hpp"
#include <array>

#ifdef MULTITHREAD
#include <thread>
#include <vector>
#endif

int main(int argc, char** argv){

    constexpr int quant = 1;
    std::array<BasicAgent, quant> team;

#ifdef MULTITHREAD

    std::vector<std::thread> threads;
    for(int i = 1; i <= quant; ++i) {

        threads.emplace_back([&team, i]() {
            while (true) {
                if (team[i - 1].run()) {
                    return 0;
                }
            }
        });
    }

    // Espera todas as threads terminarem
    for(auto& t : threads) {
        t.join();
    }
#else
    // Funcionamento Contínuo
    while(true) {
        for(int i = 1; i <= quant; ++i) {
            if(team[i - 1].run()) {
                return 0;
            }
        }
    }
#endif
    return 0;
}