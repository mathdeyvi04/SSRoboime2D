#include "./booting/cxxopts.hpp"
#include "./agent/TrainerAgent.hpp"

int main(int argc, char* argv[]) {

    /* -- Parsing de Possibilidades -- */

    // Criamos o parser do nosso binário
    cxxopts::Options options(
        "",
        "Executável do tipo ELF responsável por prover à RoboIME um gerenciamento do treinamento dos jogadores."
    );

    // Definimos as possibilidades
    options.add_options()
    (
        "s,speed",
        "Velocidade da Simulação",
        cxxopts::value<float>()
            ->default_value("1")
    )
    (
        "h,help",
        "Mostrar esta mensagem que está lida"
    );

    // A partir da matriz de possibilidades acima, realizamos o parsing
    cxxopts::ParseResult result = options.parse(argc, argv);

    // Realizamos algumas verificações
    if(result.count("help")) {
        std::cout << options.help() << "\n";
        return 0;
    }


    // Considerando que estamos treinando, estaremos em ambiente controlado por nós
    // Logo, localhost
    TrainerAgent trainer {"127.0.0.1", 6001, true};
    while(true) {

        trainer.update_params();
        if(trainer.run()) {
            return 0;
        }
    }

    return 0;
}