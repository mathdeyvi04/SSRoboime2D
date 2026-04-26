#pragma once

#include <string_view>
#include <cstdint>

#include <cstdlib>
#include <ctime>

#include "../communication/ServerComm.hpp"
#include "../environment/Environment.hpp"
#include "../booting/TacticalFormations.hpp"


class BasicAgent {
private:

    // Portal de Comunicações entre jogador e servidor
    ServerComm __sc;

    // Painel de Variáveis de Mundo
    Environment __env;

public:

    BasicAgent() {
        // Inicializamos todas os pontos principais
       this->__env.unum = this->__sc.unum;

        // Teletransportamos o jogador para a posição correta
        this->beam(
            -54 + (rand() % 55), // Restrito ao Booting
            -32 + (rand() % 65), // Restrito ao Booting
            // Inicialmente, vamos apenas passar 0 para left e 180 para right
            0,
            0
        );
    };

    /**
     * @brief Teletransporta o agente para posição absoluta no campo. Também é capaz de
     * movimentar a cabeça do jogador
     *
     * @param posx Coordenada X (-52 a 52)
     * @param posy Coordenada Y (-34 a 34)
     * @param angle_body Ângulo do corpo (graus)
     * @param angle_head Ângulo da cabeça (graus)
     *
     * @note Executa 3 comandos: move (teletransporte), turn (corpo), turn_neck (cabeça)
     */
    void beam(int posx, int posy, int angle_body = 0, int angle_head = 0){
        // Teletransportamos o corpo
        this->__sc.send_immediate(
            std::format(
                "(move {} {})",
                posx,
                posy
            )
        );

        if(!angle_body){ return; }
        // Movemos o corpo
        this->__sc.send_immediate(
            std::format(
                "(turn {})",
                angle_body
            )
        );

        if(!angle_head){ return; }
        // Movemos a cabeça
        this->__sc.send_immediate(
            std::format(
                "(turn_neck {})",
                angle_head
            )
        );
    }

    void run() {
        // Recebemos algo do servidor
        std::string_view message_from_server = this->__sc.receive();

        // Interpretamos a mensagem
        this->__env.wp.update_from_server(message_from_server, this->__env);

        // Tomamos alguma decisão

    }


};