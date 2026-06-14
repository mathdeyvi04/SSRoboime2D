#pragma once

#include <string_view>
#include <cstdint>
#include <array>
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
    // Counter Necessário para não explodirmos o servidor de comandos
    std::array<uint8_t, 2> counter={0,8};

    // Flag de verificação de visibilidade da bola
    bool ball_is_visible = false;

#ifdef AGENT_INFO
    inline static std::array<
        float,
        /*
        - ball_is_visible
        - counter[0]
        - counter[1]
        */
        11 * 3
    > each_agent_info;
#endif

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

        if(Environment::pm == Environment::PlayMode::BEFORE_KICK_OFF) {
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

        return 1;
    }

    int seek_the_ball() {
        // Moveremos o pescoço para mantermos a cabeça olhando para a bola, sempre.

        // Atualizaremos o estado da bola
        this->ball_is_visible = false;
        for(uint8_t i = 0; i < this->__env.number_visibles; ++i){
            if(this->__env.visibles_index[i] == 0) {
                this->ball_is_visible = true;
                break;
            }
        }

        if(this->ball_is_visible) {
            // Então vamos olhar para ela
            // Neste caso em específico, sabemos que ela está em (0, 0)
            // Precisamos da nossa ângulo em graus para virar a cabeça!
        }

        return 1;
    }

    /**
     * @brief Executa um ciclo completo de Percepção, Atualização e Ação do agente.
     * * Este métodx atua como o motor do robô. Ele escuta o servidor do simulador,
     * atualiza a percepção de mundo do robô com base nas mensagens recebidas e
     * toma a decisão de buscar a bola.
     * * @return int Retorna 0 se o ciclo foi executado com sucesso;
     * Retorna 1 se a conexão com o servidor foi encerrada.
     */
    int run() {
        if(this->__sc.isclosed()) {
            return 1;
        }
        // Recebemos algo do servidor
        std::string_view message_from_server = this->__sc.receive();

        // Interpretamos a mensagem
        this->__env.wp.update_from_server(message_from_server, this->__env);

        // Tomamos uma decisão
        this->seek_the_ball();

#ifdef AGENT_INFO
        // Populamos o vetor de informações

        BasicAgent::each_agent_info[3 * (__env.unum - 1)] = this->ball_is_visible;
        BasicAgent::each_agent_info[3 * (__env.unum - 1) + 1] = this->counter[0];
        BasicAgent::each_agent_info[3 * (__env.unum - 1) + 2] = this->counter[1];

#endif // AGENT_INFO

        return 0;
    }

    /**
     * @brief Altera a orientação angular do corpo e da cabeça do robô.
     * * Envia comandos síncronos/imediatos para o servidor do simulador.
     * O comando do corpo é sempre enviado, enquanto o da cabeça depende do valor ser diferente de zero.
     * @note Valores de ângulos são em graus para evitar rejeição pelo protocolo do servidor.
     * @param angle_body Ângulo desejado para rotação do corpo.
     * @param angle_head Ângulo desejado para rotação do pescoço (relativo ao corpo). Padrão é 0.
     */
    int turn(int angle_body = 0, int angle_head = 0) {

        if(this->counter[1]++ != 10) {
            return 0;
        }

        // Envio imperativo do comando de rotação do corpo
        if(angle_body != 0) {
            this->__sc.send_immediate(
                std::format(
                    "(turn {})",
                    angle_body
                )
            );
        }
        // Envio condicional do comando de rotação da cabeça
        if(angle_head != 0) {
            this->__sc.send_immediate(
                std::format(
                    "(turn_neck {})", // Corrigido para o padrão do protocolo (com parêntese)
                    angle_head
                )
            );
        }
        this->counter[1] = 0;

        return 1;
}

};