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

    constexpr uint8_t amount {11};
    std::array<BasicAgent, amount> team;

#ifdef MULTITHREAD
    std::atomic<uint8_t> amount_alive {0};
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
    constexpr uint8_t WIDTH {15};

    std::array<std::ostringstream, BasicAgent::total_attrs + 1> projetor;
    std::array<std::string, BasicAgent::total_attrs> attr_names = {
        "ball_is_visible"
    };
    while(amount_alive != 0) {

        // Title Line
        projetor[0] << std::setw(WIDTH) << "Player";
        for(uint8_t i = 0; i < amount; ++i) {
            projetor[0] << std::setw(WIDTH) << i + 1;
        }

        for(
            uint8_t idx_for_attr = 0;
            idx_for_attr < BasicAgent::total_attrs;
            ++idx_for_attr
        ) {
            // Pode ser pensado adicionarmos nomes à cada atributo
            projetor[idx_for_attr + 1] << std::setw(WIDTH) << attr_names[idx_for_attr];
            for(uint8_t j = 0; j < amount; ++j) {
                // Atributo x está na linha x + 1, logo:
                projetor[idx_for_attr + 1] << std::setw(WIDTH) << BasicAgent::each_agent_info[j * BasicAgent::total_attrs + idx_for_attr];
            }
        }

        // Apresentation
        std::cout << "\033[H";
        for(uint8_t i = 0; i < projetor.size(); ++i) {
            std::cout << projetor[i].str() << "\n";
            projetor[i].str("");
            projetor[i].clear();
        }
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