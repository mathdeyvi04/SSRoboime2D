#include "./agent/BasicAgent.hpp"
#include <array>

int main(int argc, char** argv){

    const int quant = 11;
    std::array<BasicAgent, quant> team;

    // Funcionamento Contínuo
    while(true){
        for(int i = 1; i <= quant; ++i){
            if(team[i - 1].run()) {
                return 0;
            }
        }
    }

    return 0;
}