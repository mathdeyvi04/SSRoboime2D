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
#include "../environment/Environment.hpp"
#include "../booting/TacticalFormations.hpp"


class BasicAgent {
private:
    // Portal de Comunicações entre jogador e servidor
    ServerComm __sc;

    // Painel de Variáveis de Mundo
    Environment __env;

public:

    // Flag de verificação de visibilidade da bola
    bool ball_is_visible {false};

#ifdef AGENT_INFO

    inline static total_attrs {1};
    inline static std::array<
        float,
        /*
        - ball_is_visible
        */
        11 * BasicAgent::total_attrs
    > each_agent_info {};
#endif

    // -- Proveremos as ações possíveis
    struct Dash {
        double power;  // potência do dash (positiva para frente, negativa para trás)

        [[nodiscard]]
        size_t serialize(std::array<char, 64>& buffer) const {
            char* ptr = buffer.data();

            // Prefixo "(dash "
            std::memcpy(
                ptr,      // destino
                "(dash ", // origem
                6         // tamanho do prefixo
            );
            ptr += 6;

            // Converte 'power' para string
            std::to_chars_result result = std::to_chars(
                ptr,                           // início da escrita
                buffer.data() + buffer.size(), // limite do buffer
                power                          // valor
            );
            ptr = result.ptr;

            // Fecha parênteses
            *ptr++ = ')';

            return ptr - buffer.data();
        }
    };
    struct Turn {
        double angle_body;  // ângulo de rotação (em graus, positivo para anti-horário)

        [[nodiscard]]
        size_t serialize(std::array<char, 64>& buffer) const {
            char* ptr = buffer.data();

            std::memcpy(
                ptr,
                "(turn ",
                6
            );
            ptr += 6;

            std::to_chars_result result = std::to_chars(
                ptr,
                buffer.data() + buffer.size(),
                angle_body
            );
            ptr = result.ptr;

            *ptr++ = ')';

            return ptr - buffer.data();
        }
    };
    struct TurnNeck {
        double angle_head;  // ângulo de rotação do pescoço (visão)

        [[nodiscard]]
        size_t serialize(std::array<char, 64>& buffer) const {
            char* ptr = buffer.data();

            std::memcpy(
                ptr,
                "(turn_neck ",
                11
            );
            ptr += 11;

            std::to_chars_result result = std::to_chars(
                ptr,
                buffer.data() + buffer.size(),
                angle_head
            );
            ptr = result.ptr;

            *ptr++ = ')';

            return ptr - buffer.data();
        }
    };
    struct Move {
        double x;  // coordenada X (campo, absoluta)
        double y;  // coordenada Y

        [[nodiscard]]
        size_t serialize(std::array<char, 64>& buffer) const {
            char* ptr = buffer.data();

            std::memcpy(
                ptr,
                "(move ",
                6
            );
            ptr += 6;

            // 1º argumento: x
            std::to_chars_result result = std::to_chars(
                ptr,
                buffer.data() + buffer.size(),
                x
            );
            ptr = result.ptr;

            // Espaço separador
            *ptr++ = ' ';

            // 2º argumento: y
            result = std::to_chars(
                ptr,
                buffer.data() + buffer.size(),
                y
            );
            ptr = result.ptr;

            *ptr++ = ')';

            return ptr - buffer.data();
        }
    };
    struct Say {
        std::string message;  // texto da mensagem (sem espaços, conforme protocolo)

        [[nodiscard]]
        size_t serialize(std::array<char, 64>& buffer) const {
            char* ptr = buffer.data();

            std::memcpy(
                ptr,
                "(say ",
                5
            );
            ptr += 5;

            // Copia a mensagem (sem espaços)
            std::memcpy(
                ptr,                     // destino
                message.data(),          // origem
                message.size()           // número de bytes
            );
            ptr += message.size();

            *ptr++ = ')';

            return ptr - buffer.data();
        }
    };
    struct Kick {
        double power;
        double direction;

        [[nodiscard]]
        size_t serialize(std::array<char, 64>& buffer) const {
            // Ponteiro para posição atual de escrita no buffer
            char* ptr = buffer.data();

            // Copia o prefixo do comando "(kick " para o buffer
            std::memcpy(
                ptr,           // destino
                "(kick ",      // origem
                6              // número de bytes a copiar
            );
            ptr += 6;

            // Converte o valor 'power' para string e avança o ponteiro
            std::to_chars_result result_power = std::to_chars(
                ptr,                           // início da escrita
                buffer.data() + buffer.size(), // limite do buffer
                power                          // valor a ser convertido
            );
            ptr = result_power.ptr;

            // Insere um espaço separador entre os argumentos
            *ptr++ = ' ';

            // Converte o valor 'direction' para string e avança o ponteiro
            std::to_chars_result result_direction = std::to_chars(
                ptr,                           // início da escrita
                buffer.data() + buffer.size(), // limite do buffer
                direction                      // valor a ser convertido
            );
            ptr = result_direction.ptr;

            // Adiciona o parêntese de fechamento do comando
            *ptr++ = ')';

            // Retorna o número total de bytes escritos no buffer
            return ptr - buffer.data();
        }
    };
    struct Tackle {
        double power;     // força do tackle
        double direction; // direção (ângulo) do tackle

        [[nodiscard]]
        size_t serialize(std::array<char, 64>& buffer) const {
            char* ptr = buffer.data();

            std::memcpy(
                ptr,
                "(tackle ",
                8
            );
            ptr += 8;

            // power
            std::to_chars_result result = std::to_chars(
                ptr,
                buffer.data() + buffer.size(),
                power
            );
            ptr = result.ptr;

            *ptr++ = ' ';

            // direction
            result = std::to_chars(
                ptr,
                buffer.data() + buffer.size(),
                direction
            );
            ptr = result.ptr;

            *ptr++ = ')';

            return ptr - buffer.data();
        }
    };
    struct Catch {
        double direction;  // ângulo para onde estender as mãos (para pegar a bola)

        [[nodiscard]]
        size_t serialize(std::array<char, 64>& buffer) const {
            char* ptr = buffer.data();

            std::memcpy(
                ptr,
                "(catch ",
            7
            );
            ptr += 7;

            std::to_chars_result result = std::to_chars(
                ptr,
                buffer.data() + buffer.size(),
                direction
            );
            ptr = result.ptr;

            *ptr++ = ')';

            return ptr - buffer.data();
        }
    };
    struct ChangeView {
        enum class Width { Normal, Wide, Narrow };
        Width width;

        [[nodiscard]]
        size_t serialize(std::array<char, 64>& buffer) const {
            char* ptr = buffer.data();

            std::memcpy(
                ptr,
                "(change_view ",
                13
            );
            ptr += 13;

            // Converte o enum para string
            const char* width_str = nullptr;
            switch (width) {
                case Width::Normal: width_str = "normal"; break;
                case Width::Wide:   width_str = "wide";   break;
                case Width::Narrow: width_str = "narrow"; break;
            }
            size_t len = std::strlen(width_str);
            std::memcpy(
                ptr,
                width_str,
                len
            );
            ptr += len;

            *ptr++ = ')';

            return ptr - buffer.data();
        }
    };
    using AgentAction = std::variant<
        Dash,
        Turn,
        TurnNeck,
        Move,
        Say,
        Kick,
        Tackle,
        Catch,
        ChangeView
    >;
    /**
     * @brief Fila de ações do agente a serem enviadas ao servidor.
     *
     * Armazena comandos representados por um variant contendo todos os tipos
     * de ações possíveis (Dash, Turn, Kick, etc.).
     */
    std::queue<AgentAction> command_queue {};
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
            const AgentAction& action = this->command_queue.front();


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
    void beam(double posx, double posy, int angle_body = 0, int angle_head = 0) {
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
     * - Potência baseada na stamina atual:
     *   - >6000 -> 100%
     *   - >3000 -> 60%
     *   - ≤3000 -> 20%
     */
    int dash(double power) {

        if(Environment::pm == Environment::PlayMode::BEFORE_KICK_OFF) {
            return 0;
        }

        if(Environment::cycle < 150) {
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
        this->command_queue.push(Dash{power});
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

        this->command_queue.push(Turn{angle_body});

        if (!angle_head) {
            this->command_queue.push(TurnNeck{angle_head});
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

        this->command_queue.push(Kick{power, direction});
        this->body_command_flag = true;

        return 1;  // Retorno adicionado para consistência
    }

    /**
     * @brief Busca a bola no ambiente e atualiza o estado de visibilidade.
     * Esta função verifica se a bola (índice 0) está presente entre os objetos
     * visíveis no ambiente atual. Se visível, prepara o agente para orientar-se
     * em direção à bola (posição (0,0) neste contexto específico).
     *
     * @return int Sempre retorna 1 (sucesso).
     * @note A bola é identificada pelo índice 0 no array de visíveis.
     */
    int seek_the_ball() {

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
        if(this->__sc.isclosed()) [[unlikely]] {
            this->__env.logger.info(
                std::format(
                    "Jogador {} saiu de campo.",
                    this->__env.unum
                )
            );
            return 1;
        }
        // Recebemos algo do servidor
        std::string_view message_from_server = this->__sc.receive();

        // Interpretamos a mensagem
        this->__env.wp.update_from_server(message_from_server, this->__env);

        // Antes de tomarmos as decisões, devemos resetar algumas coisas
        this->body_command_flag = false;

        // Tomamos decisões
        this->dash(100);

        // Enviamos os comandos
        this->send_commands();

#ifdef AGENT_INFO
        // Populamos o vetor de informações

        uint8_t idx = BasicAgent::total_attrs * (__env.unum - 1);
        BasicAgent::each_agent_info[idx] = this->ball_is_visible;

#endif // AGENT_INFO
        return 0;
    }
};