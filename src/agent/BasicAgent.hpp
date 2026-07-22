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
public:
    // Portal de Comunicações entre jogador e servidor
    ServerComm m_sc;

    // Painel de Variáveis de Mundo
    Environment m_env;

    // Localizer
    Localizer m_loc;

    // Flag de verificação de visibilidade da bola
    bool m_ball_is_visible {false};

    // Permitir a apresentação de informações
    bool m_verbose {false};

    /** @brief Número de atributos monitorados por agente */
    inline static constexpr int TOTAL_ATTRS {5};
    /** @brief Matriz 1D de atributos de todos os agentes (11 x TOTAL_ATTRS) */
    inline static std::array<
        float,
        /*
        - m_ball_is_visible
        - posx
        - posy
        - body_angle
        - head_angle
        */
        11 * BasicAgent::TOTAL_ATTRS
    > EACH_AGENT_INFO {};

    ////////////////////////////////////////
    /* -- Funções de Envio de Comandos -- */
    ////////////////////////////////////////

    /**
     * @brief Fila de ações do agente a serem enviadas ao servidor.
     *
     * Armazena comandos representados por um variant contendo todos os tipos
     * de ações possíveis (Dash, Turn, Kick, etc.).
     */
    std::queue<BasicCommands::AgentAction> m_command_queue {};
    /* Buffer Reutilizável para Serialização dos Comandos */
    std::array<char, 64> m_command_buffer {};
    /* Os seguintes comandos não podem ser acionados ao mesmo tempo: Dash, Turn, Kick */
    bool m_body_command_flag {false};

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

        std::array<char, 64>& buffer = m_command_buffer;
        while(!m_command_queue.empty()) {
            // Obtém referência para o próximo comando da fila
            const BasicCommands::AgentAction& action = m_command_queue.front();


            // Serializa o comando para o buffer usando std::visit
            // O visit resolve o tipo em tempo de compilação
            size_t bytes_escritos = std::visit(
                [&buffer](const auto& real_action)
                {
                    return real_action.serialize(buffer);
                },
                action
            );

            m_sc.send_immediate(
                buffer.data(),
                bytes_escritos
            );
            m_command_queue.pop();
        }

        // Quando servidor é executado no modo síncrono, todos precisam enviar esta mensagem
        m_sc.send_immediate("(done)");
    }

    /////////////////////////////
    /* -- Definições Básicas --*/
    /////////////////////////////

    BasicAgent(
        const std::string& team_name,
        const std::string& ip,
        int port,
        bool verbose
    ) :
        m_sc{team_name, ip, port}
    {
        // Inicializamos pontos principais
       m_env.m_unum = m_sc.m_unum;
       m_env.m_team_name = std::move(team_name);
       m_verbose = verbose;
       m_env.m_verbose = verbose;

        // Teletransportamos o jogador para a posição correta
        beam(
            TacticalFormations::Default[2 * m_env.m_unum - 2], // Restrito ao Booting
            TacticalFormations::Default[2 * m_env.m_unum - 2 + 1], // Restrito ao Booting
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
        m_sc.send_immediate(
            std::format(
                "(move {} {})",
                posx,
                posy
            )
        );
        m_env.m_position[0] = posx;
        m_env.m_position[1] = posy;

        if(!angle_body){ return; }
        // Movemos o corpo
        m_sc.send_immediate(
            std::format(
                "(turn {})",
                angle_body
            )
        );

        if(!angle_head){ return; }
        // Movemos a cabeça
        m_sc.send_immediate(
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

        if(Environment::PM == Environment::PlayMode::BEFORE_KICK_OFF) {
            return 0;
        }

        if(m_body_command_flag) {
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

        if (m_env.m_stamina_info[0] <= 3000) {
            power = std::min(power, 20.0);
        }
        else if (m_env.m_stamina_info[0] <= 6000) {
            power = std::min(power, 60.0);
        }
        else {
            power = std::min(power, 100.0);
        }
        m_command_queue.push(BasicCommands::Dash{power});
        m_body_command_flag = true;

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
        if(m_body_command_flag) {
            return 0;
        }

        m_command_queue.push(BasicCommands::Turn{angle_body});

        if(angle_head != 0) {
            m_command_queue.push(BasicCommands::TurnNeck{angle_head});
        }
        m_body_command_flag = true;
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
        if(m_body_command_flag) {
            return 0;
        }

        m_command_queue.push(BasicCommands::Kick{power, direction});
        m_body_command_flag = true;

        return 1;  // Retorno adicionado para consistência
    }

    /**
     * @brief Processa mensagens do servidor para o agente e Atualiza o estado de atributos
     *
     * Bloqueia até receber uma mensagem válida, depois drena o buffer
     * de mensagens pendentes (non-blocking) antes de retornar.
     * Atualiza diversas informações como visualização da bola e localização.
     *
     * @return int 0 em sucesso, 1 se conexão for encerrada.
     */
    int perception_and_update() {

        std::string_view message_from_server {};
        // Aguarda até receber uma mensagem válida
        while(true) {

            if(m_sc.isclosed()) {
                if(m_verbose) {
                    m_env.m_logger.info(
                        std::format(
                            "Jogador {} saiu de campo.",
                            m_env.m_unum
                        )
                    );
                }
                return 1;
            }
            // Tenta receber dados do servidor (modo bloqueante)
            message_from_server = m_sc.receive(false);
            if(message_from_server.empty()) {
                // Timeout temporário, tenta novamente
                continue;
            }
            // Mensagem recebida com sucesso
            break;
        }

        // Processa a primeira mensagem recebida
        m_env.m_wp.update_from_server(
            message_from_server,
            m_env
        );

        // Loop não-bloqueante: drena o buffer do socket
        while(true) {

            if(m_sc.isclosed()) {
                if(m_verbose) {
                    m_env.m_logger.info(
                        std::format(
                            "Jogador {} saiu de campo.",
                            m_env.m_unum
                        )
                    );
                }
                return 1;
            }
            // Tenta receber dados sem bloquear
            message_from_server = m_sc.receive(true);
            if(message_from_server.empty()) {
                // Buffer do SO está vazio, não há mais dados
                break;
            }
            // Processa cada mensagem adicional recebida
            m_env.m_wp.update_from_server(
                message_from_server,
                m_env
            );
        }

        // Antes de tomarmos as decisões, devemos reinicializar algumas variáveis
        m_body_command_flag = false;
        m_ball_is_visible   = false;
        m_loc.m_count_for_landmarks_visibles = 0;

        // Atualizamos estado de visibilidade
        for(int i = 0; i < m_env.m_number_visibles; ++i) {
            const int& index_point_visible = m_env.m_visibles_index[i];

            if(index_point_visible == 0) {
                // Bola está visível
                m_ball_is_visible = true;
                continue;
            }

            // Verificamos posições fixas em campo
            m_loc.verify_landmarks(index_point_visible);
        }

        m_loc.update_location(
            m_env.m_position,
            m_env.m_points_on_the_field
        );

        /*
        - Pode ser bom passearmos por todos os elementos visíveis de novo atualizando os correspondentes vetores posições
            Entretanto, isso abre espaço para perda de desempenho, tento em vista que passaremos por todos os pontos de novo.
            Oq eu pensei foi: podemos verificar os pontos e atualizar a posição e pose assim que possível. A partir do momento
            que tivermos o primeiro valor, podemos apenas aplicá-lo aos próximos elementos visíveis, atualizando as respectivas
            posições. "Ah mas e os anteriores a quando tiver a posição e pose?" Será o preço pago...
        */

        return 0;
    }

    //////////////////////////////////////////////////////////////
    /* -- Funções Não Triviais -- */
    //////////////////////////////////////////////////////////////

    int Seek(
        double posx,
        double posy
    ) {

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

        if(perception_and_update()) {
            return 1;
        }

        ///////////////////////////////////////////////////////////////////
        /* -- Ação -- */
        ///////////////////////////////////////////////////////////////////









        ///////////////////////////////////////////////////////////////////
        /* -- Envio de Decisões -- */
        ///////////////////////////////////////////////////////////////////

        // Enviamos os comandos
        send_commands();

        // Populamos o vetor de informações
        if(m_verbose) {
            int idx = BasicAgent::TOTAL_ATTRS * (m_env.m_unum - 1);
            BasicAgent::EACH_AGENT_INFO[idx] = m_ball_is_visible;
            BasicAgent::EACH_AGENT_INFO[idx + 1] = m_env.m_position[0];
            BasicAgent::EACH_AGENT_INFO[idx + 2] = m_env.m_position[1];
            BasicAgent::EACH_AGENT_INFO[idx + 3] = m_env.m_body_angle;
            BasicAgent::EACH_AGENT_INFO[idx + 4] = m_env.m_head_angle;
        }

        return 0;
    }
};