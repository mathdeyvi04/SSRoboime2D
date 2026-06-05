#pragma once

#include <string_view>
#include <cstdint>
#include <array>
#include <cstdlib>
#include <ctime>

#include <iostream>
#include <ranges>

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
    // Counter Necessário para não explodirmos o servidor de comandos
    std::array<uint8_t, 1> counter;

    bool ball_is_visible = false;

    BasicAgent() {
        // Inicializamos todas os pontos principais
       this->__env.unum = this->__sc.unum;

        // Teletransportamos o jogador para a posição correta
        this->beam(
            TacticalFormations::Default[2 * this->__env.unum - 2], // Restrito ao Booting
            TacticalFormations::Default[2 * this->__env.unum - 2 + 1], // Restrito ao Booting
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
    void beam(double posx, double posy, int angle_body = 0, int angle_head = 0){
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

    /**
     * @brief Executa ação de correr (dash) com potência adaptativa.
     * @return 1 Se o dash foi executado.
     * @return 0 Se ainda não é momento de dash (pré-jogo ou contador não atingido).
     *
     * @details
     * - Não age durante o modo BEFORE_KICK_OFF.
     * - Executa apenas a cada N chamadas (contador[0] módulo N).
     * - Potência baseada na stamina atual:
     *   - >6000 -> 100%
     *   - >3000 -> 60%
     *   - ≤3000 -> 20%
     */
    int dash() {

        if(Environment::pm == Environment::PlayMode::BEFORE_KICK_OFF){
            return 0;
        }

        if(this->counter[0]++ != 10) {
            return 0;
        }

        /* Deve-se fazer juízo de 3 fatores para a utilização dessa função:
        - Urgência Tática
            Dependendo do que está acontecendo na partida
        - Distância à bola ou jogador específico
            Caso a bola esteja suficientemente perto do jogador
        - Stamina_info
            Apenas uma função de actual_stamina, effort e capacity. Esses valores afetam
            drasticamente o dash e é necessário um gerenciamento de energia inteligente.

        À primeira vista, lidaremos apenas com stamina_info.
        */

        uint8_t power = (this->__env.stamina_info[0] > 6000) ? 100 : (
                                                             (this->__env.stamina_info[0] > 3000) ? 60 : 20
                                                             );
        this->__sc.send_immediate(
            std::format(
                "(dash {})",
                power
            )
        );
        this->counter[0] = 0;

//        // Apenas para verificação dos parâmetros:
//        std::cout << "stamina_info: ";
//        std::ranges::copy(this->__env.stamina_info, std::ostream_iterator<int>(std::cout, " "));
//        std::cout << std::endl;

        return 1;
    }

    int seek_the_ball() {
        // Moveremos o pescoço para mantermos a cabeça olhando para a bola, sempre.

        // Atualizaremos o estado da bola
        for(uint8_t i = 0; i < this->__env.number_visibles; ++i){
            if(this->__env.visibles_index[i] == 0) {
                this->ball_is_visible = true;
                break;
            }
        }
        this->ball_is_visible = false;

        if(this->ball_is_visible) {
            // Então vamos olhar para ela
            // Neste caso em específico, sabemos que ela está em (0, 0)
            std::cout << "Vejo a bola" << std::endl;
            // Precisamos da nossa ângulo em graus para virar a cabeça!
        }

        return 1;
    }

    int run() {
        if(this->__sc.isclosed()) {
            return 1;
        }
        // Recebemos algo do servidor
        std::string_view message_from_server = this->__sc.receive();

        // Interpretamos a mensagem
        this->__env.wp.update_from_server(message_from_server, this->__env);

        // Tomamos alguma decisão
        this->seek_the_ball();

        return 0;
    }


};