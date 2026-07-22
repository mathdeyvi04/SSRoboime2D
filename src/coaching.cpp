#include "./agent/TrainerAgent.hpp"

int main(int argc, char* argv[]) {

    // Considerando que estamos treinando, estaremos em ambiente controlado por nós
    // Logo, localhost
    TrainerAgent trainer {"127.0.0.1", 6001, true};
    while(true) {
        if(trainer.run()) {
            return 0;
        }
    }

    return 0;
}