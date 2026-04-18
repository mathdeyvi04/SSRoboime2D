#include "./agent/BasicAgent.hpp"
#include <array>

int main(int argc, char** argv){

    srand(time(nullptr));

    const int quant = 1;
    std::array<BasicAgent, quant> team;

    // Funcionamento Contínuo
    while(true){
        for(int i = 0; i < quant; ++i){
            team[i].run();
        }
    }

    return 0;
}