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
#include <chrono>
#include <thread>

#include "../communication/ServerComm.hpp"
#include "../communication/BasicCommands.hpp"
#include "../environment/Environment.hpp"
#include "../environment/Localizer.hpp"
#include "../booting/TacticalFormations.hpp"
#include "../booting/SoccerParams.hpp"
#include "../math/GeneralMath.hpp"

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

    /** @brief Temporizador entre ciclos, terá como base o valor de 100 ms */
    std::chrono::milliseconds m_target_duration;

    /** @brief Número de atributos monitorados por agente */
    inline static constexpr int TOTAL_ATTRS {7};

    /** @brief Matriz 1D de atributos de todos os agentes (11 x TOTAL_ATTRS)
     *  - m_ball_is_visible
     *  - m_position_x
     *  - m_position_y
     *  - m_confidence
     *  - m_body_angle
     *  - m_head_angle
     *  - m_value
     *  Usando essa estrutura, não precisaremos nos preocupar com índices ou ordem
     */
    inline static GeneralMath::smart_array<11 * BasicAgent::TOTAL_ATTRS> EACH_AGENT_INFO {};

    /** @brief Valor Aleatório que será utilizado quando necessário. Pense nele como aquelas flags da CPU */
    double m_value {};

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
    /* @brief Buffer Reutilizável para Serialização dos Comandos */
    std::array<char, 64> m_command_buffer {};
    /* @brief Flags de comandos corporais, Dash, Turn, Kick.
     * Isso nos permitirá maior controle da característica do servidor de não aceitar diferentes comandos.
    */
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

        // Quando servidor é executado no modo síncrono, todos precisam enviar esta mensagem
        m_command_queue.push(BasicCommands::Done{});

        // todo urgent: Deve ser provida uma maneira de impedir que comandos de corpo sejam enviados no mesmo ciclo, pode ser usado os counters e outras lógicas de priorização
        std::array<char, 64>& buffer = m_command_buffer;
        std::string message_sended_to_server {};
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

            if(m_verbose) {
                message_sended_to_server.append(std::string_view {buffer.data(), bytes_escritos});
            }
            m_sc.send_immediate(
                buffer.data(),
                bytes_escritos
            );
            m_command_queue.pop();
        }
        // Apenas quando julgar necessário
        if(m_verbose && !message_sended_to_server.empty()) {
            m_env.m_logger.info("Last Cycle Received: {} | P {} send: {}", Environment::CYCLE, m_env.m_unum, message_sended_to_server);
        }
    }

    /////////////////////////////
    /* -- Definições Básicas --*/
    /////////////////////////////

    BasicAgent(
        const std::string& team_name,
        const std::string& ip,
        int port,
        bool verbose,
        float speed = 1.1 // Desejamos que seja mais rápido que trainer
    ) :
        m_sc{team_name, ip, port},
        m_env{},
        m_loc{m_env.m_points_on_the_field},
        m_target_duration{static_cast<const unsigned long>(100 * speed)}
    {
        // Inicializamos pontos principais
       m_env.m_unum = m_sc.m_unum;
       m_env.m_team_name = std::move(team_name);
       m_verbose = verbose;
       m_env.m_verbose = verbose;

        // Teletransportamos o jogador para a posição correta
        beam(
            TacticalFormations::Default[2 * m_env.m_unum - 2], // Restrito ao Booting
            TacticalFormations::Default[2 * m_env.m_unum - 2 + 1] // Restrito ao Booting
        );
    }

    /**
     * @brief Teletransporta o agente para posição absoluta no campo. Também é capaz de
     * movimentar a cabeça do jogador. Essa função somente é executada uma vez.
     * @param posx Coordenada X (-52 a 52)
     * @param posy Coordenada Y (-34 a 34)
     * @note Executa 3 comandos: move (teletransporte), turn (corpo), turn_neck (cabeça)
     */
    void beam(double posx, double posy) {
        m_command_queue.push(
            BasicCommands::Move {posx, posy}
        );
        m_env.m_position[0] = posx;
        m_env.m_position[1] = posy;
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
            const int index_point_visible = m_env.m_visibles_index[i];

            if(index_point_visible == 0) {
                // Bola está visível
                m_ball_is_visible = true;
                continue;
            }

            // Verificamos posições fixas em campo
            if(index_point_visible > 1 && index_point_visible < 56) {
                m_loc.verify_landmarks(index_point_visible);
            }
        }

        // todo urgent: Ainda não há uma forma de atualizarmos a posição sem vermos os landmarks
        int duration = m_loc.update_location(
            m_env.m_position,
            m_env.m_points_on_the_field
        );
        if(duration > 0) {
            m_env.m_logger.warn("Landmark Counter, Localizer Duration:{},{}", m_loc.m_confidence, duration);
        }
        // Isso é uma aproximação MUITO SUPERIOR
        m_env.m_body_angle = GeneralMath::normalize_angle(m_env.m_position[2] * 180 / GeneralMath::PI - m_env.m_head_angle);

        /*
        - Pode ser bom passearmos por todos os elementos visíveis de novo atualizando os correspondentes vetores posições
            Entretanto, isso abre espaço para perda de desempenho, tento em vista que passaremos por todos os pontos de novo.
            Oq eu pensei foi: podemos verificar os pontos e atualizar a posição e pose assim que possível. A partir do momento
            que tivermos o primeiro valor, podemos apenas aplicá-lo aos próximos elementos visíveis, atualizando as respectivas
            posições. "Ah mas e os anteriores a quando tiver a posição e pose?" Será o preço pago...
        */

        return 0;
    }

    ////////////////////////////////
    /* -- Funções Não Triviais -- */
    ////////////////////////////////

    /** @brief Conforme descoberto nos estudos sobre o comando Turn, faz-se necessário uma correção do valor passado */
    double get_turn_correction() {
        return 1.05806 + 4.53338 * m_env.m_speed[0];
    }

    /**
     * @brief Função Auxiliar para Seek_and_Focus: Ângulos (em graus) para cada índice, calculados a partir dos tokens
     * direcionais segundo a convenção do servidor:
     *  - bottom (b) -> y positivo (+90°)
     *  - top    (t) -> y negativo (-90°)
     *  - left   (l) -> x negativo (180°)
     *  - right  (r) -> x positivo (0°)
     *  - center (c) -> (0,0)
     *
     * O primeiro token de cada sequência é ignorado (categoria), os demais
     * são somados vetorialmente. O ângulo resultante é arredondado para
     * múltiplos de 45°.
     */
    static constexpr std::array<double, 60> DIRECTION_ANGLES = {
         0.0,    // 0   "b"
        90.0,    // 1   "fb0"
       135.0,    // 2   "fbl1"
       135.0,    // 3   "fbl2"
       135.0,    // 4   "fbl3"
       135.0,    // 5   "fbl4"
       135.0,    // 6   "fbl5"
        45.0,    // 7   "fbr1"
        45.0,    // 8   "fbr2"
        45.0,    // 9   "fbr3"
        45.0,    // 10  "fbr4"
        45.0,    // 11  "fbr5"
         0.0,    // 12  "fc"
        90.0,    // 13  "fcb"
       -90.0,    // 14  "fct"
       135.0,    // 15  "fglb"
      -135.0,    // 16  "fglt"
        45.0,    // 17  "fgrb"
       -45.0,    // 18  "fgrt"
       180.0,    // 19  "fl0"
       135.0,    // 20  "flb"
       135.0,    // 21  "flb1"
       135.0,    // 22  "flb2"
       135.0,    // 23  "flb3"
      -135.0,    // 24  "flt"
      -135.0,    // 25  "flt1"
      -135.0,    // 26  "flt2"
      -135.0,    // 27  "flt3"
       135.0,    // 28  "fplb"
       180.0,    // 29  "fplc"
      -135.0,    // 30  "fplt"
        45.0,    // 31  "fprb"
         0.0,    // 32  "fprc"
       -45.0,    // 33  "fprt"
         0.0,    // 34  "fr0"
        45.0,    // 35  "frb"
        45.0,    // 36  "frb1"
        45.0,    // 37  "frb2"
        45.0,    // 38  "frb3"
       -45.0,    // 39  "frt"
       -45.0,    // 40  "frt1"
       -45.0,    // 41  "frt2"
       -45.0,    // 42  "frt3"
       -90.0,    // 43  "ft0"
      -135.0,    // 44  "ftl1"
      -135.0,    // 45  "ftl2"
      -135.0,    // 46  "ftl3"
      -135.0,    // 47  "ftl4"
      -135.0,    // 48  "ftl5"
       -45.0,    // 49  "ftr1"
       -45.0,    // 50  "ftr2"
       -45.0,    // 51  "ftr3"
       -45.0,    // 52  "ftr4"
       -45.0,    // 53  "ftr5"
       180.0,    // 54  "gl"
         0.0,    // 55  "gr"
        90.0,    // 56  "lb"
       180.0,    // 57  "ll"
         0.0,    // 58  "lr"
       -90.0     // 59  "lt"
    };
    /** @brief Counter para contarmos quantas vezes estamos usando informações desatualizadas */
    int m_use_outdated_information_seek_and_focus {};
    /**
     * @brief Direciona a visão do agente (corpo e/ou pescoço) para um ponto desejado.
     * @details Não aplica as mudanças dos ângulos às variáveis. Ele espera o retorno da função `update_location` e da mensagem `see`
     * do servidor para atualizar os valores de body_angle e head_angle.
     *
     * @param index_in_points_on_the_field  Índice do ponto no campo (1-59 para pontos fixos,
     *                                      qualquer outro valor para busca dinâmica; 0 ou negativo
     *                                      para bola/jogadores ou coordenadas manuais).
     * @param force_full_body               Se true, o agente gira completamente o corpo para o alvo,
     *                                      zerando o ângulo do pescoço. Padrão false.
     * @param posx_to_focus                 Coordenada X de foco quando nenhum ponto é encontrado (padrão 99.0).
     * @param posy_to_focus                 Coordenada Y de foco quando nenhum ponto é encontrado (padrão 99.0).
     * @return 0 sempre (sucesso).
     */
    int Seek_and_Focus(
        int index_in_points_on_the_field,
        bool force_full_body = false,
        double posx_to_focus = 99.0,
        double posy_to_focus = 99.0
    ) {

        bool is_initialized {false};
        double angle_relative {0};
        if(index_in_points_on_the_field >= 1 && index_in_points_on_the_field <= 59) {
            /*
            Então é um elemento real do campo e FIXO, o que é vantajoso.
            */
            angle_relative = BasicAgent::DIRECTION_ANGLES[index_in_points_on_the_field] - (m_env.m_head_angle + m_env.m_body_angle);
            m_use_outdated_information_seek_and_focus = 0;
            is_initialized = true;
        }
        if(index_in_points_on_the_field >= 0 && !is_initialized) {
            /*
            Então pode ser a bola ou demais jogadores
            */
            for(int i = 0; i < m_env.m_number_visibles; ++i) {
                if(index_in_points_on_the_field == m_env.m_visibles_index[i]) {
                    // Então está visível
                    // O campo [1] de attrs de point já fornecerá o ângulo relativo.
                    Environment::Point& point = m_env.m_points_on_the_field[index_in_points_on_the_field];
                    angle_relative = point.attrs[1];
                    m_use_outdated_information_seek_and_focus = 0;
                    is_initialized = true;
                    break;
                }
            }
            if(!is_initialized) {
                /*
                Então não está visível, utilizaremos a última informação visual dele.
                Caso fiquemos travados nessa informação visual desatualizada, vamos forçar giros.
                */
                Environment::Point& point = m_env.m_points_on_the_field[index_in_points_on_the_field];
                angle_relative = point.attrs[1] + (m_use_outdated_information_seek_and_focus++) * 30;
                is_initialized = true;
            }
        }
        if(!is_initialized) {
            // Então é apenas um ponto no campo
            angle_relative = GeneralMath::angle_of_vector(m_env.m_position[0] - posx_to_focus, m_env.m_position[1] - posy_to_focus);
            angle_relative = GeneralMath::normalize_angle(
                angle_relative - (m_env.m_head_angle + m_env.m_body_angle)
            );
            m_use_outdated_information_seek_and_focus = 0;
            is_initialized = true;
        }

        // Caso esteja abaixo deste limite, então nem devemos fazer nada
        if(std::abs(angle_relative) < Agent::MIN_ANGLE_TO_TURN_NECK) {
            return 0;
        }

        // todo lazy: Observe que consideramos que todos os comandos foram bem sucedidos.
        angle_relative *= Agent::PARAM_TO_TURN_NECK_ON_SEEK_AND_FOCUS;

        // Conforme o jogador se movimenta, o ângulo muda efetivamente girado é diferente.
        // Inclusive, dependendo do jogador, o momento pode mudar também. Estes foram os valores que obtivemos para o id 0.
        double TURN_CORRECTION = get_turn_correction();

        // Se forçado, gira o corpo totalmente para o alvo e zera o pescoço
        if(force_full_body) {
            double target_body_angle = GeneralMath::normalize_angle(m_env.m_body_angle + m_env.m_head_angle + angle_relative);
            double body_turn = GeneralMath::normalize_angle(target_body_angle - m_env.m_body_angle);
            if(std::abs(body_turn) > Agent::MIN_ANGLE_TO_TURN_NECK) {
                m_command_queue.push(BasicCommands::Turn{body_turn * TURN_CORRECTION});
                m_body_command_flag = true;
            }
            if(std::abs(m_env.m_head_angle) > Agent::MIN_ANGLE_TO_TURN_NECK) {
                m_command_queue.push(BasicCommands::TurnNeck{-m_env.m_head_angle});
            }
            return 0;
        }

        // Caso o ponto esteja tenha um ângulo relativo grande demais
        if(std::abs(angle_relative) > 90) {
            m_command_queue.push(
                BasicCommands::Turn{
                    angle_relative * TURN_CORRECTION
                }
            );
            m_body_command_flag = true;
            if(std::abs(m_env.m_head_angle) > Agent::MIN_ANGLE_TO_TURN_NECK) {
                m_command_queue.push(
                    BasicCommands::TurnNeck{
                        - m_env.m_head_angle
                    }
                );
            };
            return 0;
        }

        // Caso o ângulo entre a cabeça e o corpo esteja acima de um limiar
        if(std::abs(m_env.m_head_angle) > Agent::MIN_DIF_ANGLE_TO_BODY_FOLLOW_HEAD && !m_body_command_flag) {
            m_command_queue.push(
                BasicCommands::Turn{
                    m_env.m_head_angle * TURN_CORRECTION
                }
            );
            m_command_queue.push(
                BasicCommands::TurnNeck{
                    - m_env.m_head_angle + angle_relative
                }
            );
            m_body_command_flag = true;
            return 0;
        }

        // Execução normal
        m_command_queue.push(
            BasicCommands::TurnNeck{
                angle_relative
            }
        );
        return 0;
    }

    int Walk() {
        // TER UMA VARIÁVEL INDICANDO SE A BOLA ESTÁ COM SEU TIME OU NÃO
        // Se estiver a menos de uma distância mínima, anda até a bola
        // Se estiver a menos de uma outra distância mínima, se posiciona entre o adversário e a bola
        // Se nenhum dos anteriores, fica parado
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

        if(perception_and_update()) {
            return 1;
        }
        auto start_time = std::chrono::steady_clock::now();

        ///////////////////////////////////////////////////////////////////
        /* -- Ação -- */
        ///////////////////////////////////////////////////////////////////

        /* Controle de Movimento Básico */

        // Devemos impedir que ele saía de campo.
        if(m_loc.check_if_out_of_field(m_env.m_position, m_env.m_body_angle)) {
            m_command_queue.push(BasicCommands::Turn{GeneralMath::normalize_angle(180 * get_turn_correction())});
            m_body_command_flag = true; // Como é o primeiro body movement, não precisamos nos preocupar com isso
        }

        if(
            Environment::CYCLE > 0 &&
            static_cast<int>(m_value) % 12 == 0
        ) {
            Seek_and_Focus(static_cast<int>(m_value / 10), true);
            if(m_value > 60 * 10) {
                m_value = 0;
            }
        }
        if(Environment::CYCLE > 0) {
            if(!m_body_command_flag) {
                m_command_queue.push(BasicCommands::Dash{80});
                m_body_command_flag = true;
            }
        }
        m_value++;

        ///////////////////////////////////////////////////////////////////
        /* -- Envio de Decisões -- */
        ///////////////////////////////////////////////////////////////////

        // Enviamos os comandos
        send_commands();

        // Populamos o vetor de informações
        if(m_verbose) {
            int idx = BasicAgent::TOTAL_ATTRS * (m_env.m_unum - 1);
            BasicAgent::EACH_AGENT_INFO.set(idx, m_ball_is_visible);
            BasicAgent::EACH_AGENT_INFO.set(idx, m_env.m_position[0]);
            BasicAgent::EACH_AGENT_INFO.set(idx, m_env.m_position[1]);
            BasicAgent::EACH_AGENT_INFO.set(idx, m_loc.m_confidence);
            BasicAgent::EACH_AGENT_INFO.set(idx, m_env.m_body_angle);
            BasicAgent::EACH_AGENT_INFO.set(idx, m_env.m_head_angle);
            BasicAgent::EACH_AGENT_INFO.set(idx, m_value);
// Para a realização de testes interessantes
            m_env.m_logger.error("{},{},{},{},{},{},{},{}",
                                                   Environment::CYCLE,
                                                   m_env.m_position[0],
                                                   m_env.m_position[1],
                                                   m_env.m_position[2] * 180 / 3.141592,
                                                   m_loc.m_confidence,
                                                   m_env.m_body_angle,
                                                   m_env.m_head_angle,
                                                   m_env.m_speed[0]
                                 );
        }

        auto end_time = std::chrono::steady_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        if(elapsed_time < m_target_duration) {
            std::this_thread::sleep_for(m_target_duration - elapsed_time);
        }
        return 0;
    }
};