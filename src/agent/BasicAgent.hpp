#pragma once

#include <string_view>
#include <cstdint>
#include <array>
#include <cstdlib>
#include <ctime>
#include <queue>
#include <charconv>
#include <variant>
#include <cstring>
#include <string>
#include <memory>

#include "../communication/ServerComm.hpp"
#include "../communication/BasicCommands.hpp"
#include "../environment/Environment.hpp"
#include "../environment/Localizer.hpp"
#include "../booting/TacticalFormations.hpp"


class BasicAgent {
private:
    // Portal de Comunicações entre jogador e servidor
    ServerComm __sc;

    // Painel de Variáveis de Mundo
    Environment __env;

    // Localizer
    Localizer __loc;

public:

    // Flag de verificação de visibilidade da bola
    bool ball_is_visible {false};

    // Permitir a apresentação de informações
    bool verbose {false};

    /** @brief Número de atributos monitorados por agente */
    inline static constexpr int total_attrs {3};
    /** @brief Matriz 1D de atributos de todos os agentes (11 x total_attrs) */
    inline static std::array<
        float,
        /*
        - ball_is_visible
        - posx
        - posy
        */
        11 * BasicAgent::total_attrs
    > each_agent_info {};

    /**
     * @brief Fila de ações do agente a serem enviadas ao servidor.
     *
     * Armazena comandos representados por um variant contendo todos os tipos
     * de ações possíveis (Dash, Turn, Kick, etc.).
     */
    std::queue<BasicCommands::AgentAction> command_queue {};
    /* Buffer Reutilizável para Serialização dos Comandos */
    std::array<char, 64> command_buffer {};
    /* Os seguintes comandos não podem ser acionados ao mesmo tempo: Dash, Turn, Kick */
    bool body_command_flag {false};

    /**
     * @brief Envia todos os comandos enfileirados para o servidor.
     *
     * Percorre a fila de ações, serializa cada comando no buffer interno
     * e transmite via UDP utilizando o socket configurado.
     *
     * @note Os comandos são enviados em ordem FIFO (First-In, First-Out).
     * @note Cada comando é removido da fila após o envio bem-sucedido.
     */
    void send_commands() {

        std::array<char, 64>& buffer = this->command_buffer;
        while(!this->command_queue.empty()) {
            // Obtém referência para o próximo comando da fila
            const BasicCommands::AgentAction& action = this->command_queue.front();


            // Serializa o comando para o buffer usando std::visit
            // O visit resolve o tipo em tempo de compilação
            size_t bytes_escritos = std::visit(
                [&buffer](const auto& real_action)
                {
                    return real_action.serialize(buffer);
                },
                action
            );

            this->__sc.send_immediate(
                buffer.data(),
                bytes_escritos
            );
            command_queue.pop();
        }
    }

    BasicAgent(
        const std::string& ip,
        int port,
        bool verbose
    ) :
        __sc{ip, port}
    {
        // Inicializamos pontos principais
       this->__env.unum = this->__sc.unum;
       this->verbose = verbose;

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
    void beam(double posx, double posy, double angle_body = 0, double angle_head = 0) {
        // Teletransportamos o corpo
        this->__sc.send_immediate(
            std::format(
                "(move {} {})",
                posx,
                posy
            )
        );
        this->__env.position[0] = posx;
        this->__env.position[1] = posy;

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
     * - Potência baseada na stamina atual:
     *   - >6000 -> 100%
     *   - >3000 -> 60%
     *   - ≤3000 -> 20%
     */
    int dash(double power) {

        if(Environment::pm == Environment::PlayMode::BEFORE_KICK_OFF) {
            return 0;
        }

        if(this->body_command_flag) {
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

        if (this->__env.stamina_info[0] <= 3000) {
            power = std::min(power, 20.0);
        }
        else if (this->__env.stamina_info[0] <= 6000) {
            power = std::min(power, 60.0);
        }
        else {
            power = std::min(power, 100.0);
        }
        this->command_queue.push(BasicCommands::Dash{power});
        this->body_command_flag = true;

        return 1;
    }

    /**
     * @brief Executa rotação do corpo e opcionalmente do pescoço.
     *
     * @param angle_body Ângulo de rotação do corpo em graus (positivo = anti-horário).
     * @param angle_head Ângulo de rotação do pescoço em graus (padrão = 0).
     *
     * @return int 1 se o comando foi enfileirado com sucesso, 0 se o corpo já possui comando pendente.
     *
     * @note Apenas um comando corporal pode ser enfileirado por ciclo.
     * @note Se angle_head não for fornecido, nenhum comando de pescoço é enfileirado.
     */
    int turn(
        double angle_body,
        double angle_head = 0
    ) {
        if(this->body_command_flag) {
            return 0;
        }

        this->command_queue.push(BasicCommands::Turn{angle_body});

        if (!angle_head) {
            this->command_queue.push(BasicCommands::TurnNeck{angle_head});
        }

        this->body_command_flag = true;
        return 1;
    }

    /**
     * @brief Executa um chute na bola com potência e direção especificadas.
     *
     * @param power Potência do chute (0-100).
     * @param direction Direção do chute em graus relativa à orientação atual do corpo.
     *
     * @return int 1 se o comando foi enfileirado com sucesso, 0 se o corpo já possui comando pendente.
     *
     * @note Apenas um comando corporal pode ser enfileirado por ciclo.
     * @note A direção é relativa ao corpo do agente, não absoluta no campo.
     */
    int kick(
        double power,
        double direction
    ) {
        if(this->body_command_flag) {
            return 0;
        }

        this->command_queue.push(BasicCommands::Kick{power, direction});
        this->body_command_flag = true;

        return 1;  // Retorno adicionado para consistência
    }

    /**
     * @brief Processa todas as mensagens pendentes do servidor.
     * @return int 0 se bem-sucedido, 1 se o socket foi fechado.
     * @details
     * 1. Bloqueia até receber a primeira mensagem (timeout)
     * 2. Processa a mensagem recebida
     * 3. Esvazia o buffer do socket com recepções não-bloqueantes
     * 4. Processa todas as mensagens em fila
     */
    int perception_and_update() {

        std::string_view message_from_server {};
        // Aguarda até receber uma mensagem válida
        while(true) {

            if(this->__sc.isclosed()) {
                this->__env.logger.info(
                    std::format(
                        "Jogador {} saiu de campo.",
                        this->__env.unum
                    )
                );
                return 1;
            }
            // Tenta receber dados do servidor (modo bloqueante)
            message_from_server = this->__sc.receive(false);
            if(message_from_server.empty()) {
                continue;  // Timeout temporário, tenta novamente
            }
            break;  // Mensagem recebida com sucesso
        }

        // Processa a primeira mensagem recebida
        this->__env.wp.update_from_server(
            message_from_server,
            this->__env
        );

        // Loop não-bloqueante: drena o buffer do socket
        // Processa todas as mensagens pendentes
        while(true) {

            if(this->__sc.isclosed()) {
                this->__env.logger.info(
                    std::format(
                        "Jogador {} saiu de campo.",
                        this->__env.unum
                    )
                );
                return 1;
            }
            // Tenta receber dados sem bloquear
            message_from_server = this->__sc.receive(true);
            if(message_from_server.empty()) {
                break;  // Buffer do SO está vazio, não há mais dados
            }
            // Processa cada mensagem adicional recebida
            this->__env.wp.update_from_server(
                message_from_server,
                this->__env
            );
        }
        return 0;
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

        ///////////////////////////////////////////////////////////////////
        /* -- Percepção e Atualização -- */
        ///////////////////////////////////////////////////////////////////

        if(this->perception_and_update()) {
            return 1;
        }

        ///////////////////////////////////////////////////////////////////
        /* -- Ação -- */
        ///////////////////////////////////////////////////////////////////

        // Antes de tomarmos as decisões, devemos resetar algumas coisas
        this->body_command_flag = false;
        this->ball_is_visible   = false;
        this->__loc.count_for_landmarks_visibles = 0;

        // Atualizamos estado de visibilidade
        for(int i = 0; i < this->__env.number_visibles; ++i) {
            const int& index_point_visible = this->__env.visibles_index[i];

            if(index_point_visible == 0) {
                // Bola está vísivel
                this->ball_is_visible = true;
            }

            // Verificamos posições fixas em campo
            this->__loc.verify_landmarks(index_point_visible);
        }

        this->__loc.update_location(
            this->__env.position,
            this->__env.points_on_the_field
        );

        ///////////////////////////////////////////////////////////////////
        /* -- Envio de Decisões -- */
        ///////////////////////////////////////////////////////////////////

        // Enviamos os comandos
        this->send_commands();

        // Populamos o vetor de informações
        if(this->verbose) {
            int idx = BasicAgent::total_attrs * (__env.unum - 1);
            BasicAgent::each_agent_info[idx] = this->ball_is_visible;
            BasicAgent::each_agent_info[idx + 1] = this->__env.position[0];
            BasicAgent::each_agent_info[idx + 2] = this->__env.position[1];
        }

        return 0;
    }
};