#include "./agent/BasicAgent.hpp"
#include <array>

#ifdef MULTITHREAD
    #include <thread>
    #include <vector>

#ifdef AGENT_INFO
    #include <iomanip>
    #include <sstream>
    #include <atomic>

#endif // AGENT_INFO
#endif // MULTITHREAD

int main(int argc, char** argv){

    constexpr uint8_t amount = 3;
    std::array<BasicAgent, amount> team;

#ifdef MULTITHREAD

    std::atomic<uint8_t> amount_alive = 0;
    std::vector<std::thread> threads;
    for(uint8_t i = 1; i <= amount; ++i) {

        amount_alive++;
        threads.emplace_back([&team, &amount_alive, i]() {
            while (true) {
                if (team[i - 1].run()) {
                    amount_alive--;
                    return 0;
                }
            }
        });
    }

#ifdef AGENT_INFO
    // Apresentação da tela
    std::cout << "\033[2J";
    constexpr uint8_t WIDTH = 12;

    std::array<std::ostringstream, BasicAgent::each_agent_info.size() / 11 + 1> projetor;
    while(amount_alive != 0) {

        // Title Line
        projetor[0] << std::setw(WIDTH) << "Player";
        for(uint8_t i = 0; i < amount; ++i) {
            projetor[0] << std::setw(WIDTH) << i + 1;
        }

        for(
            uint8_t idx_for_attr = 0;
            idx_for_attr < static_cast<uint8_t>(BasicAgent::each_agent_info.size() / 11);
            ++idx_for_attr
        ) {
            projetor[idx_for_attr + 1] << std::setw(WIDTH) << "      ";
            for(uint8_t j = 0; j < amount; ++j) {
                // Atributo x está na linha x + 1, logo:
                projetor[idx_for_attr + 1] << std::setw(WIDTH) << BasicAgent::each_agent_info[j * 3 + idx_for_attr];
            }
        }

        // Apresentation
        std::cout << "\033[H";
        for(uint8_t i = 0; i < (amount + 1); ++i) {
            std::cout << projetor[i].str() << "\n";
            projetor[i].str("");
        }
        std::cout << std::endl;;
    }
#endif // AGENT_INFO

    // Espera todas as threads terminarem
    for(auto& t : threads) {
        t.join();
    }
#else
    // Funcionamento Contínuo
    while(true) {
        for(uint8_t i = 1; i <= amount; ++i) {
            if(team[i - 1].run()) {
                return 0;
            }
        }
    }
#endif // MULTITHREAD
    return 0;
}