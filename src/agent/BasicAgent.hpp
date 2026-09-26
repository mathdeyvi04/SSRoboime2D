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
#include <algorithm>
#include <cmath>
#include <limits>

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

    // Ativa uma estratégia determinística para validar as primitivas básicas.
    bool m_basic_actions_test {false};
    int m_basic_test_stage {};
    int m_basic_test_stage_cycle {-1};

    /** @brief Temporizador entre ciclos, terá como base o valor de 100 ms */
    std::chrono::milliseconds m_target_duration;

    /** @brief Matriz 1D de atributos de todos os agentes (TEAM_SIZE x TOTAL_ATTRS)
     *  - m_ball_is_visible
     *  - m_position_x
     *  - m_position_y
     *  - m_body_angle
     *  - m_head_angle
     *  - m_value
     *  Usando essa estrutura, não precisaremos nos preocupar com índices ou ordem
     */
    inline static GeneralMath::smart_array<
        Agent::TEAM_SIZE * Agent::TOTAL_ATTRS
    > EACH_AGENT_INFO {};

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
    /* Buffer Reutilizável para Serialização dos Comandos */
    std::array<char, 64> m_command_buffer {};
    /* Os seguintes comandos não podem ser acionados ao mesmo tempo: Dash, Turn, Kick */
    bool m_body_command_flag {false};

    /** Ciclo visual local deste agente e último ciclo em que cada objeto apareceu. */
    int m_latest_see_cycle {Agent::NEVER_SEEN};
    int m_last_action_see_cycle {Agent::NEVER_SEEN};
    std::array<int, Agent::POINT_COUNT> m_last_seen_cycle = [] {
        std::array<int, Agent::POINT_COUNT> cycles {};
        cycles.fill(Agent::NEVER_SEEN);
        return cycles;
    }();

    /** Estimativa absoluta mais recente da bola e de sua velocidade por ciclo. */
    std::array<double, 2> m_ball_position {};
    std::array<double, 2> m_ball_velocity {};
    int m_ball_state_cycle {Agent::NEVER_SEEN};

    /** Sentido usado pela varredura visual. */
    double m_search_direction {1.0};

    void register_see_cycle(std::string_view message) {
        constexpr std::string_view prefix {"(see "};
        if(!message.starts_with(prefix)) {
            return;
        }

        int cycle {Agent::NEVER_SEEN};
        const char* first = message.data() + prefix.size();
        const auto result = std::from_chars(first, message.data() + message.size(), cycle);
        if(result.ec == std::errc {} && result.ptr != first) {
            m_latest_see_cycle = std::max(m_latest_see_cycle, cycle);
        }
    }

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
                const size_t printable_size = bytes_escritos > 0
                    ? bytes_escritos - 1
                    : 0;
                if(!message_sended_to_server.empty()) {
                    message_sended_to_server.push_back(' ');
                }
                message_sended_to_server.append(
                    std::string_view {buffer.data(), printable_size}
                );
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
        float speed = 2.0, // Desejamos que seja mais rápido que trainer
        bool basic_actions_test = false
    ) :
        m_sc{team_name, ip, port},
        m_target_duration{
            static_cast<std::chrono::milliseconds::rep>(
                rcss::server::simulator_step * speed
            )
        }
    {
        // Inicializamos pontos principais
       m_env.m_unum = m_sc.m_unum;
       m_env.m_team_name = std::move(team_name);
       m_verbose = verbose;
       m_env.m_verbose = verbose;
       m_basic_actions_test = basic_actions_test;

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
        register_see_cycle(message_from_server);
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
            register_see_cycle(message_from_server);
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
            m_loc.verify_landmarks(index_point_visible);
        }

        // todo urgent: Ainda não há uma forma de atualizarmos a posição sem vermos os landmarks
        const bool location_updated = m_loc.update_location(
            m_env.m_position,
            m_env.m_points_on_the_field
        ) == 0;
        if(location_updated && std::isfinite(m_env.m_position[2])) {
            // O localizador estima a orientação absoluta da cabeça em radianos.
            // O comando turn, porém, usa o corpo em graus.
            const double head_absolute_degrees =
                m_env.m_position[2] * 180.0 / GeneralMath::PI;
            m_env.m_body_angle = GeneralMath::normalize_angle(
                head_absolute_degrees - m_env.m_head_angle
            );
        }

        // Converte as observações do último quadro para coordenadas absolutas.
        for(int i = 0; i < m_env.m_number_visibles; ++i) {
            const int index = m_env.m_visibles_index[i];
            if(index < 0 || index >= Agent::POINT_COUNT) {
                continue;
            }

            Environment::Point& point = m_env.m_points_on_the_field[index];
            const double absolute_direction = GeneralMath::normalize_angle(
                m_env.m_body_angle + m_env.m_head_angle + point.attrs[1]
            );
            point.pos_cart_abs[0] = m_env.m_position[0]
                + point.attrs[0] * GeneralMath::cosd(absolute_direction);
            point.pos_cart_abs[1] = m_env.m_position[1]
                + point.attrs[0] * GeneralMath::sind(absolute_direction);
            m_last_seen_cycle[index] = m_latest_see_cycle;
        }

        if(m_ball_is_visible) {
            const auto& ball = m_env.m_points_on_the_field[Agent::BALL_INDEX];
            const auto& observed = ball.pos_cart_abs;
            if(
                m_ball_state_cycle != Agent::NEVER_SEEN
                && m_latest_see_cycle > m_ball_state_cycle
            ) {
                const double elapsed = static_cast<double>(m_latest_see_cycle - m_ball_state_cycle);
                m_ball_velocity[0] = (observed[0] - m_ball_position[0]) / elapsed;
                m_ball_velocity[1] = (observed[1] - m_ball_position[1]) / elapsed;
            }
            else if(std::abs(ball.attrs[2]) < 90.0 && std::abs(ball.attrs[3]) < 90.0) {
                const double bearing = m_env.m_body_angle + m_env.m_head_angle + ball.attrs[1];
                const double angular_change = ball.attrs[3] * GeneralMath::PI / 180.0;
                const double player_direction = m_env.m_body_angle + m_env.m_speed[1];
                m_ball_velocity[0] =
                    m_env.m_speed[0] * GeneralMath::cosd(player_direction)
                    + ball.attrs[2] * GeneralMath::cosd(bearing)
                    - ball.attrs[0] * GeneralMath::sind(bearing) * angular_change;
                m_ball_velocity[1] =
                    m_env.m_speed[0] * GeneralMath::sind(player_direction)
                    + ball.attrs[2] * GeneralMath::sind(bearing)
                    + ball.attrs[0] * GeneralMath::cosd(bearing) * angular_change;
            }
            m_ball_position = observed;
            m_ball_state_cycle = m_latest_see_cycle;
        }

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
    /* -- API enxuta do agente -- */
    ////////////////////////////////

    /**
     * Uma única consulta agrega os dados necessários sobre um objeto. Isso
     * substitui os antigos métodos separados de visibilidade, idade, distância
     * e direção.
     */
    struct TargetInfo {
        bool valid {false};
        bool visible {false};
        int age {std::numeric_limits<int>::max()};
        double distance {std::numeric_limits<double>::quiet_NaN()};
        double body_direction {std::numeric_limits<double>::quiet_NaN()};
        std::array<double, 2> position {
            std::numeric_limits<double>::quiet_NaN(),
            std::numeric_limits<double>::quiet_NaN()
        };
    };

    TargetInfo target_info(int index) const {
        TargetInfo info {};
        if(
            index < 0
            || index >= Agent::POINT_COUNT
            || m_last_seen_cycle[index] == Agent::NEVER_SEEN
        ) {
            return info;
        }

        info.valid = true;
        info.age = m_latest_see_cycle == Agent::NEVER_SEEN
            ? std::numeric_limits<int>::max()
            : std::max(0, m_latest_see_cycle - m_last_seen_cycle[index]);
        for(int i = 0; i < m_env.m_number_visibles; ++i) {
            if(m_env.m_visibles_index[i] == index) {
                info.visible = true;
                break;
            }
        }

        const auto& target = m_env.m_points_on_the_field[index];
        info.position = target.pos_cart_abs;
        if(info.visible) {
            info.distance = target.attrs[0];
            info.body_direction = GeneralMath::normalize_angle(
                target.attrs[1] + m_env.m_head_angle
            );
        }
        else {
            const double dx = info.position[0] - m_env.m_position[0];
            const double dy = info.position[1] - m_env.m_position[1];
            info.distance = std::hypot(dx, dy);
            const double absolute_direction =
                std::atan2(dy, dx) * 180.0 / GeneralMath::PI;
            info.body_direction = GeneralMath::normalize_angle(
                absolute_direction - m_env.m_body_angle
            );
        }
        return info;
    }

    // COMANDOS

    bool Push_Turn(double moment) {
        if(m_body_command_flag || !std::isfinite(moment)) {
            return false;
        }
        moment = std::clamp(moment, rcss::server::minmoment, rcss::server::maxmoment);
        m_command_queue.push(BasicCommands::Turn{moment});
        m_body_command_flag = true;

        const double actual_turn = moment / (
            1.0 + rcss::server::inertia_moment * std::max(0.0, m_env.m_speed[0])
        );
        m_env.m_body_angle = GeneralMath::normalize_angle(
            m_env.m_body_angle + actual_turn
        );
        return true;
    }

    bool Push_Dash(double power) {
        if(m_body_command_flag || !std::isfinite(power)) {
            return false;
        }
        power = std::clamp(power, rcss::server::minpower, rcss::server::maxpower);
        m_command_queue.push(BasicCommands::Dash{power});
        m_body_command_flag = true;
        return true;
    }

    bool Push_Kick(double power, double direction) {
        if(m_body_command_flag || !std::isfinite(power) || !std::isfinite(direction)) {
            return false;
        }
        m_command_queue.push(BasicCommands::Kick{
            std::clamp(power, rcss::server::minpower, rcss::server::maxpower),
            GeneralMath::normalize_angle(direction)
        });
        m_body_command_flag = true;
        return true;
    }

    bool Push_Turn_Neck(double moment) {
        if(!std::isfinite(moment)) {
            return false;
        }
        const double desired_head = std::clamp(
            m_env.m_head_angle + moment,
            -rcss::server::maxneckang,
            rcss::server::maxneckang
        );
        const double allowed_moment = std::clamp(
            desired_head - m_env.m_head_angle,
            -rcss::server::maxneckmoment,
            rcss::server::maxneckmoment
        );
        if(std::abs(allowed_moment) < GeneralMath::EPSILON) {
            return false;
        }
        m_command_queue.push(BasicCommands::TurnNeck{allowed_moment});
        m_env.m_head_angle = desired_head;
        return true;
    }

    // VISÃO

    bool Search(int index = Agent::BALL_INDEX, double sweep_angle = 45.0) {
        if(target_info(index).visible) {
            return true;
        }

        sweep_angle = std::clamp(std::abs(sweep_angle), 1.0, 90.0);
        const double next_head = m_env.m_head_angle + m_search_direction * sweep_angle;
        if(std::abs(next_head) <= rcss::server::maxneckang) {
            return Push_Turn_Neck(m_search_direction * sweep_angle);
        }

        m_search_direction *= -1.0;
        const double desired_turn = m_search_direction * sweep_angle;
        const double moment = desired_turn * (
            1.0 + rcss::server::inertia_moment * std::max(0.0, m_env.m_speed[0])
        );
        const bool turned = Push_Turn(moment);
        if(std::abs(m_env.m_head_angle) >= Agent::MIN_ANGLE_TO_TURN_NECK) {
            Push_Turn_Neck(-m_env.m_head_angle);
        }
        return turned;
    }

    bool Seek_and_Focus(
        int index,
        bool force_full_body = false,
        double fallback_x = 99.0,
        double fallback_y = 99.0
    ) {
        const TargetInfo target = target_info(index);
        double relative_to_body {};
        if(target.valid) {
            relative_to_body = target.body_direction;
        }
        else if(fallback_x != 99.0 || fallback_y != 99.0) {
            const double dx = fallback_x - m_env.m_position[0];
            const double dy = fallback_y - m_env.m_position[1];
            relative_to_body = GeneralMath::normalize_angle(
                std::atan2(dy, dx) * 180.0 / GeneralMath::PI - m_env.m_body_angle
            );
        }
        else {
            return Search(index);
        }

        const double relative_to_head = GeneralMath::normalize_angle(
            relative_to_body - m_env.m_head_angle
        );
        if(
            !force_full_body
            && std::abs(relative_to_head) >= Agent::MIN_ANGLE_TO_TURN_NECK
            && std::abs(m_env.m_head_angle + relative_to_head)
                <= rcss::server::maxneckang
        ) {
            return Push_Turn_Neck(relative_to_head);
        }

        bool turned = true;
        if(std::abs(relative_to_body) >= Agent::MIN_ANGLE_TO_TURN_NECK) {
            const double moment = relative_to_body * (
                1.0 + rcss::server::inertia_moment * std::max(0.0, m_env.m_speed[0])
            );
            turned = Push_Turn(moment);
        }
        if(std::abs(m_env.m_head_angle) >= Agent::MIN_ANGLE_TO_TURN_NECK) {
            Push_Turn_Neck(-m_env.m_head_angle);
        }
        return turned;
    }

    // MOVIMENTO

    bool Walk(double x, double y) {
        const double dx = x - m_env.m_position[0];
        const double dy = y - m_env.m_position[1];
        const double distance = std::hypot(dx, dy);
        if(distance <= 0.75) {
            return true;
        }
        if(m_body_command_flag) {
            return false;
        }

        const double direction = GeneralMath::normalize_angle(
            std::atan2(dy, dx) * 180.0 / GeneralMath::PI - m_env.m_body_angle
        );
        if(std::abs(direction) > 10.0) {
            const double moment = direction * (
                1.0 + rcss::server::inertia_moment * std::max(0.0, m_env.m_speed[0])
            );
            return Push_Turn(moment);
        }

        const double effort = m_env.m_stamina_info[1] > 0.0
            ? m_env.m_stamina_info[1]
            : rcss::server::effort_init;
        double power = std::max(0.0, distance - 0.5)
            / (rcss::server::dash_power_rate * effort);
        const double stamina = m_env.m_stamina_info[0];
        if(stamina > 0.0 && stamina < 0.25 * rcss::server::stamina_max) {
            power = std::min(power, 35.0);
        }
        else if(stamina > 0.0 && stamina < 0.50 * rcss::server::stamina_max) {
            power = std::min(power, 65.0);
        }
        return Push_Dash(std::clamp(power, 0.0, rcss::server::max_dash_power));
    }

    bool Walk(int index) {
        const TargetInfo target = target_info(index);
        if(!target.valid) {
            return false;
        }
        if(target.distance <= 0.75) {
            return true;
        }
        if(m_body_command_flag) {
            return false;
        }
        if(std::abs(target.body_direction) > 10.0) {
            const double moment = target.body_direction * (
                1.0 + rcss::server::inertia_moment * std::max(0.0, m_env.m_speed[0])
            );
            return Push_Turn(moment);
        }

        const double effort = m_env.m_stamina_info[1] > 0.0
            ? m_env.m_stamina_info[1]
            : rcss::server::effort_init;
        double power = std::max(0.0, target.distance - 0.5)
            / (rcss::server::dash_power_rate * effort);
        const double stamina = m_env.m_stamina_info[0];
        if(stamina > 0.0 && stamina < 0.25 * rcss::server::stamina_max) {
            power = std::min(power, 35.0);
        }
        else if(stamina > 0.0 && stamina < 0.50 * rcss::server::stamina_max) {
            power = std::min(power, 65.0);
        }
        return Push_Dash(std::clamp(power, 0.0, rcss::server::max_dash_power));
    }

    // BOLA

    bool kick_to(double x, double y) {
        const TargetInfo ball = target_info(Agent::BALL_INDEX);
        const double kickable_distance =
            rcss::server::player_size
            + rcss::server::ball_size
            + rcss::server::kickable_margin;
        if(!ball.visible || ball.distance > kickable_distance) {
            return false;
        }

        const double dx = x - ball.position[0];
        const double dy = y - ball.position[1];
        const double target_distance = std::hypot(dx, dy);
        const double direction = GeneralMath::normalize_angle(
            std::atan2(dy, dx) * 180.0 / GeneralMath::PI - m_env.m_body_angle
        );
        const double desired_speed = std::min(
            rcss::server::ball_speed_max,
            target_distance * (1.0 - rcss::server::ball_decay)
        );
        const double angle_penalty =
            0.25 * std::abs(GeneralMath::normalize_angle(
                direction - ball.body_direction
            )) / 180.0;
        const double contact_distance = std::max(
            0.0,
            ball.distance - rcss::server::player_size - rcss::server::ball_size
        );
        const double distance_penalty =
            0.25 * contact_distance / rcss::server::kickable_margin;
        const double effectiveness =
            std::max(0.1, 1.0 - angle_penalty - distance_penalty);
        const double power = std::clamp(
            desired_speed / (rcss::server::kick_power_rate * effectiveness),
            0.0,
            rcss::server::maxpower
        );
        return Push_Kick(power, direction);
    }

    bool kick_to(int target_index) {
        const TargetInfo target = target_info(target_index);
        if(!target.valid) {
            return false;
        }
        return kick_to(target.position[0], target.position[1]);
    }

    bool Intercept_Ball(int max_prediction_cycles = 30) {
        const TargetInfo ball = target_info(Agent::BALL_INDEX);
        const double kickable_distance =
            rcss::server::player_size
            + rcss::server::ball_size
            + rcss::server::kickable_margin;
        if(ball.visible && ball.distance <= kickable_distance) {
            return true;
        }
        if(ball.visible) {
            return Walk(Agent::BALL_INDEX);
        }
        if(!ball.valid || ball.age > Agent::MAX_STALE_BALL_CYCLES) {
            Search(Agent::BALL_INDEX);
            return false;
        }

        std::array<double, 2> interception = m_ball_position;
        for(int cycle = 0; cycle <= std::max(0, max_prediction_cycles); ++cycle) {
            const double travelled_factor = cycle == 0
                ? 0.0
                : (1.0 - std::pow(rcss::server::ball_decay, cycle))
                    / (1.0 - rcss::server::ball_decay);
            const std::array<double, 2> candidate {
                m_ball_position[0] + m_ball_velocity[0] * travelled_factor,
                m_ball_position[1] + m_ball_velocity[1] * travelled_factor
            };
            const double reachable_distance =
                kickable_distance + rcss::server::player_speed_max * cycle;
            if(
                std::hypot(
                    candidate[0] - m_env.m_position[0],
                    candidate[1] - m_env.m_position[1]
                ) <= reachable_distance
            ) {
                interception = candidate;
                break;
            }
            interception = candidate;
        }
        return Walk(interception[0], interception[1]);
    }


    /**
     * Cenário determinístico de integração das primitivas básicas.
     * Cada número de camisa recebe um papel diferente para que um único ensaio
     * exercite geometria, percepção, visão, movimento, comandos e bola.
     */
    void Run_Basic_Actions_Test() {
        const int cycle = std::max(0, m_latest_see_cycle);
        const TargetInfo ball = target_info(Agent::BALL_INDEX);
        const double kickable_distance =
            rcss::server::player_size
            + rcss::server::ball_size
            + rcss::server::kickable_margin;
        const bool ball_is_kickable =
            ball.visible && ball.distance <= kickable_distance;

        if(m_verbose && cycle % 20 == static_cast<int>(m_env.m_unum) % 20) {
            m_env.m_logger.info(
                "BASIC_TEST cycle={} player={} stage={} ball_visible={} ball_age={} "
                "ball_distance={} ball_body_direction={} body_available={}",
                cycle,
                m_env.m_unum,
                m_basic_test_stage,
                ball.visible,
                ball.age,
                ball.distance,
                ball.body_direction,
                !m_body_command_flag
            );
        }

        // Antes do início da partida, ainda podemos validar a varredura visual.
        if(Environment::PM != Environment::PlayMode::PLAY_ON) {
            Search(Agent::BALL_INDEX);
            return;
        }

        const auto advance_stage = [this, cycle]() {
            ++m_basic_test_stage;
            m_basic_test_stage_cycle = cycle;
        };
        const int stage_age = m_basic_test_stage_cycle < 0
            ? std::numeric_limits<int>::max()
            : cycle - m_basic_test_stage_cycle;

        switch(m_env.m_unum) {
            case 1: {
                // Busca, intercepta e passa para o jogador 2 (índice aliado 61).
                constexpr int RECEIVER_INDEX {61};
                const TargetInfo receiver = target_info(RECEIVER_INDEX);
                if(m_basic_test_stage == 0) {
                    if(!ball_is_kickable) {
                        Intercept_Ball();
                    }
                    else if(receiver.visible) {
                        if(kick_to(RECEIVER_INDEX)) {
                            advance_stage();
                        }
                    }
                    else {
                        Seek_and_Focus(RECEIVER_INDEX);
                    }
                }
                else {
                    Seek_and_Focus(RECEIVER_INDEX);
                }
                return;
            }

            case 2:
                // Recebe/intercepta o passe e finaliza no centro do gol direito.
                if(m_basic_test_stage == 0) {
                    if(!ball_is_kickable) {
                        Intercept_Ball();
                    }
                    else if(kick_to(52.5, 0.0)) {
                        advance_stage();
                    }
                }
                else {
                    if(std::abs(m_env.m_head_angle) >= Agent::MIN_ANGLE_TO_TURN_NECK) {
                        Push_Turn_Neck(-m_env.m_head_angle);
                    }
                }
                return;

            case 3: {
                // Patrulha entre dois pontos, exercitando distância e chegada.
                const std::array<double, 2> target = m_basic_test_stage % 2 == 0
                    ? std::array<double, 2>{-5.0, 12.0}
                    : std::array<double, 2>{10.0, 12.0};
                if(std::hypot(
                    target[0] - m_env.m_position[0],
                    target[1] - m_env.m_position[1]
                ) <= 1.0) {
                    advance_stage();
                }
                else {
                    Walk(target[0], target[1]);
                }
                return;
            }

            case 4: {
                // Segue o jogador 3 pelo índice observado (aliado 3 -> índice 62).
                constexpr int PLAYER_THREE_INDEX {62};
                const TargetInfo player_three = target_info(PLAYER_THREE_INDEX);
                if(player_three.visible || (player_three.valid && player_three.age <= 5)) {
                    Walk(PLAYER_THREE_INDEX);
                }
                else {
                    Search(PLAYER_THREE_INDEX);
                }
                return;
            }

            case 5:
                // Alterna o foco entre a bola e o gol, centralizando a cabeça.
                if(cycle % 40 == 39) {
                    if(std::abs(m_env.m_head_angle) >= Agent::MIN_ANGLE_TO_TURN_NECK) {
                        Push_Turn_Neck(-m_env.m_head_angle);
                    }
                }
                else if(cycle % 40 < 20) {
                    Seek_and_Focus(Agent::BALL_INDEX);
                }
                else {
                    Seek_and_Focus(55, false, 52.5, 0.0);
                }
                return;

            case 6:
                // Exercita diretamente turn, dash, turn_neck e recentralização.
                if(m_basic_test_stage == 0 && Push_Turn(45.0 * (
                    1.0 + rcss::server::inertia_moment * std::max(0.0, m_env.m_speed[0])
                ))) {
                    advance_stage();
                }
                else if(m_basic_test_stage == 1 && stage_age >= 10
                        && Push_Dash(rcss::server::max_dash_power)) {
                    advance_stage();
                }
                else if(m_basic_test_stage == 2 && stage_age >= 10
                        && Push_Turn_Neck(60.0)) {
                    advance_stage();
                }
                else if(m_basic_test_stage == 3 && stage_age >= 10
                        && (
                            std::abs(m_env.m_head_angle) < Agent::MIN_ANGLE_TO_TURN_NECK
                            || Push_Turn_Neck(-m_env.m_head_angle)
                        )) {
                    advance_stage();
                }
                else if(m_basic_test_stage >= 4 && stage_age >= 20) {
                    m_basic_test_stage = 0;
                    m_basic_test_stage_cycle = cycle;
                }
                return;

            default: {
                // Jogadores extras ocupam faixas distintas e mantêm foco na bola.
                const double lane_y = -24.0 + 6.0 * (static_cast<int>(m_env.m_unum) - 7);
                if(std::hypot(
                    -2.0 - m_env.m_position[0],
                    lane_y - m_env.m_position[1]
                ) > 1.0) {
                    Walk(-2.0, lane_y);
                }
                else {
                    Seek_and_Focus(Agent::BALL_INDEX);
                }
                return;
            }
        }
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

        if(
            m_basic_actions_test
            && m_latest_see_cycle > m_last_action_see_cycle
        ) {
            m_last_action_see_cycle = m_latest_see_cycle;
            Run_Basic_Actions_Test();
        }
        m_value++;



        ///////////////////////////////////////////////////////////////////
        /* -- Envio de Decisões -- */
        ///////////////////////////////////////////////////////////////////

        // Enviamos os comandos
        send_commands();

        // Populamos o vetor de informações
        if(m_verbose) {
            int idx = Agent::TOTAL_ATTRS * (m_env.m_unum - 1);
            BasicAgent::EACH_AGENT_INFO.set(idx, m_ball_is_visible);
            BasicAgent::EACH_AGENT_INFO.set(idx, m_env.m_position[0]);
            BasicAgent::EACH_AGENT_INFO.set(idx, m_env.m_position[1]);
            BasicAgent::EACH_AGENT_INFO.set(idx, m_env.m_body_angle);
            BasicAgent::EACH_AGENT_INFO.set(idx, m_env.m_head_angle);
            BasicAgent::EACH_AGENT_INFO.set(idx, static_cast<int>(m_value));
        }

        auto end_time = std::chrono::steady_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        if(elapsed_time < m_target_duration) {
            std::this_thread::sleep_for(m_target_duration - elapsed_time);
        }
        return 0;
    }
};
