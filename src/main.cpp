#include <array>
#include <vector>
#include <memory>
#include <thread>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <atomic>
#include "./agent/BasicAgent.hpp"
#include "./booting/cxxopts.hpp"

int main(int argc, char** argv) {

    /* -- Parsing de Possibilidades -- */

    // Criamos o parser do nosso binário
    cxxopts::Options options(
        "",
        "Executável do tipo ELF responsável por prover à RoboIME uma equipe de jogadores apta ao ambiente de simulação futebolístico 2D provido pelo rcssserver."
    );

    // Definimos as possibilidades
    options.add_options()
    (
        "t,team_name",
        "Nome do Time",
        cxxopts::value<std::string>()
            ->default_value("RoboIME")
    )
    (
        "p,players",
        "Número de jogadores (1-11)",
        cxxopts::value<int>()
            ->default_value("11")
    )
    (
        "i,ip",
        "Endereço IPv4 do Servidor",
        cxxopts::value<std::string>()
            ->default_value("127.0.0.1")
    )
    (
        "r,port",
        "Porta de Acesso ao Servidor (1-65535)",
        cxxopts::value<int>()
            ->default_value("6000")
    )
    (
        "m,multithread",
        "Permitir execução em MultiThreading",
        cxxopts::value<bool>()
            ->default_value("false")
    )
    (
        // Substittuirá o agent_info
        "v,verbose",
        "Mostrar informações extras",
        cxxopts::value<bool>()
            ->default_value("false")
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

    // Pegamos o nome do time
    const std::string& team_name = result["team_name"].as<std::string>();

    // Validação do número de jogadores
    const int amount = result["players"].as<int>();

    if(amount < 1 || amount > 11) {
        throw std::runtime_error(
            "Time deve ter um total de jogadores maior que 1 e menor que 11."
        );
    }

    // Validação do endereço IPv4
    const std::string& ip = result["ip"].as<std::string>();

    static const std::regex ip_pattern(
        R"(^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$)"
    );

    if(!std::regex_match(ip, ip_pattern)) {
        throw std::runtime_error(
            "Endereço IPv4 inválido."
        );
    }

    // Validação da porta
    const int port = result["port"].as<int>();

    if(port < 1024 || port > 65535) {
        throw std::runtime_error(
            "Porta Inválida ou Privilegiada."
        );
    }

    bool is_multithread = result["multithread"].as<bool>();
    bool verbose = result["verbose"].as<bool>();

    if(verbose && !is_multithread) {
        std::cout << "Houve um erro de interpretação, o modo `verbose` só é possível com o modo `multithread`" << std::endl;
        verbose = false;
    }

    //////////////////////////////////////////////////
    /* -- Execução de Equipe em Modo Operacional -- */
    //////////////////////////////////////////////////

    std::array<std::unique_ptr<BasicAgent>, 11> team {}; // Iniciamos com máxima quantidade possível

    for(int i = 0; i < amount; ++i) {
        team[i] = std::make_unique<BasicAgent>(team_name, ip, port, verbose);
    }

    if(!is_multithread) {

        //////////////////////////////////////////
        // Executamos como single-thread
        //////////////////////////////////////////
        while(true) {

            for(int i = 0; i < amount; ++i) {

                if(team[i]->run()) {
                    // Quando o primeiro quebrar conexão, todos os demais serão cortados.
                    return 0;
                }
            }
        } // return anterior já é a finalização
    }

    //////////////////////////////////////////
    // Executamos como multi-thread
    //////////////////////////////////////////
    std::atomic<int> amount_alive {0};
    std::vector<std::thread> workers {};
    for(int i = 0; i < amount; ++i) {

        amount_alive++;
        workers.emplace_back(
            [&team, &amount_alive, i]() {

                while(true) {

                    if(team[i]->run()) {
                        // Caso um quebre conexão, os demais continuam
                        amount_alive--;
                        return 0;
                    }
                }
            }
        );
    }

    if(verbose) {
        // Apresentaremos diversas informações de cada jogador

        std::cout << "\033[2J";
        constexpr int WIDTH {15};

        std::array<std::ostringstream, BasicAgent::TOTAL_ATTRS + 1> projetor;
        std::array<std::string, BasicAgent::TOTAL_ATTRS> attr_names = {
            "ball_is_visible",
            "position_x",
            "position_y",
            "body_angle",
            "head_angle",
            "m_value"
        };

        while(amount_alive != 0) {

            // Linha de Título
            projetor[0] << std::setw(WIDTH) << "Player";
            for(int i = 0; i < amount; ++i) {

                projetor[0] << std::setw(WIDTH) << i + 1;
            }

            // Acessamos os atributos a serem apresentados
            // E percorremos os jogadores vendo esse atributo
            for(
                int idx_for_attr = 0;
                idx_for_attr < BasicAgent::TOTAL_ATTRS;
                ++idx_for_attr
            ) {

                projetor[idx_for_attr + 1] << std::setw(WIDTH) << attr_names[idx_for_attr];
                for(int j = 0; j < amount; ++j) {

                    projetor[idx_for_attr + 1] << std::setw(WIDTH) << BasicAgent::EACH_AGENT_INFO[j * BasicAgent::TOTAL_ATTRS + idx_for_attr];
                }
            }

            // Apresentação na Tela
            std::cout << "\033[H";
            for(
                // A primeira linha é constante, mas para conseguirmos limpar a tela
                // de forma simples, preferiu-se assim
                int i = 0;
                i < static_cast<int>(projetor.size());
                ++i
            ) {

                std::cout << projetor[i].str() << "\n";

                // Limpando
                projetor[i].str("");
                projetor[i].clear();
            }
        }
    }

    for(std::thread& t: workers) {
        t.join();
    }

    return 0;
}