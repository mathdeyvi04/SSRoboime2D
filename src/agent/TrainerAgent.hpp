#pragma once

#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <sys/select.h>
#include <unistd.h>
#include <termios.h>
#include "../communication/ServerComm.hpp"

class TrainerAgent {
private:

    /** @brief Forma de Comunicação com Servidor */
    ServerComm m_sc;

    /** @brief Temporizador entre ciclos, terá como base o valor de 100 ms */
    std::chrono::milliseconds m_target_duration;

    /** @brief Acumula bytes do stdin até que uma linha completa (\n) seja formada */
    std::string m_input_buffer {};

    /** @brief Guardará as últimas mensagens do servidor
     * [0] -> Delay entre ciclos
     * [1] -> see_global
     * [2] -> última mensagem do servidor que não é see_global
     */
    std::array<std::string, 3> m_menu_info {};

    /** @brief Variável apenas para armazenar estado do buffer */
    termios m_original_terminal_config {};

public:

    TrainerAgent(
        const std::string& ip,
        int port,
        float speed
    ) :
        m_sc{"", ip, port, true},
        m_target_duration{static_cast<const unsigned long>(speed * 100)}
    {
        /*
         * Salva a configuração original do terminal.
         */
        tcgetattr(
            STDIN_FILENO,
            &m_original_terminal_config
        );

        termios raw = m_original_terminal_config;

        /*
         * Desativa o modo canônico.
         *
         * read() passa a retornar caractere por caractere,
         * sem esperar pelo '\n'.
         */
        raw.c_lflag &= ~ICANON;

        /*
         * Desativa o echo automático.
         *
         * O terminal não exibirá automaticamente os caracteres.
         * A aplicação fará isso manualmente.
         */
        raw.c_lflag &= ~ECHO;

        /*
         * VMIN = 1:
         * read() retorna após pelo menos um caractere.
         */
        raw.c_cc[VMIN] = 1;

        /*
         * Como usamos select() antes de read(), o read()
         * não deverá bloquear.
         */
        raw.c_cc[VTIME] = 0;

        tcsetattr(
            STDIN_FILENO,
            TCSANOW,
            &raw
        );
    }

    ~TrainerAgent() {
        /*
         * Restaura o terminal ao estado original.
         */
        tcsetattr(
            STDIN_FILENO,
            TCSANOW,
            &m_original_terminal_config
        );

        /*
         * Garante que a próxima saída do shell comece
         * em uma nova linha.
         */
        std::cout << '\n';
    }

    /**
     * @brief Verifica entrada do usuário e processa comandos.
     *
     * Lê teclas pressionadas em modo não-canônico:
     * - ESC: encerra execução
     * - ENTER: envia mensagem acumulada
     * - BACKSPACE: remove último caractere
     * - Outras: acumula na mensagem
     *
     * @return int 0 se mensagem enviada, 1 se deseja sair ou sem entrada.
     */
    int check_and_send() {

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        /*
         * Timeout zerado:
         *
         * select() retorna imediatamente.
         */
        timeval timeout{0, 0};

        int ready = select(
            STDIN_FILENO + 1,
            &read_fds,
            nullptr,
            nullptr,
            &timeout
        );

        if(ready <= 0) {
            return 1;
        }

        char c;

        /*
         * Como o terminal está em modo não-canônico,
         * read() recebe caracteres individualmente.
         */
        ssize_t n = read(
            STDIN_FILENO,
            &c,
            1
        );

        if(n <= 0) {
            return 1;
        }

        /*
         * ESC (ASCII 27):
         *
         * encerra a execução do agente.
         */
        if(c == 27) {

            /*
             * Opcionalmente, você pode limpar o menu
             * antes de sair.
             */
            std::cout << "\033[2J\033[H"
                      << std::flush;

            return 1;
        }

        /*
         * ENTER:
         *
         * envia a mensagem acumulada.
         */
        if(c == '\n' || c == '\r') {

            if(!m_input_buffer.empty()) {

                m_sc.send_immediate(
                    m_input_buffer
                );

                m_input_buffer.clear();
            }

            return 0;
        }

        /*
         * BACKSPACE:
         *
         * ASCII 127 é o valor mais comum.
         */
        if(c == 127 || c == '\b') {

            if(!m_input_buffer.empty()) {

                m_input_buffer.pop_back();

                /*
                 * Remove visualmente o último caractere:
                 *
                 * \b -> volta
                 * espaço -> apaga
                 * \b -> volta novamente
                 */
                std::cout << "\b \b"
                          << std::flush;
            }

            return 0;
        }

        /*
         * Qualquer caractere normal:
         *
         * 1. armazena;
         * 2. exibe visualmente.
         */
        m_input_buffer += c;

        std::cout << c
                  << std::flush;

        return 0;
    }

    /**
     * @brief Exibe o menu interativo no terminal.
     *
     * Limpa a tela, mostra as opções e o prompt com a entrada atual.
     *
     * @return int 0 em sucesso.
     */
    int print_menu() {

        system("clear");

        for(int i = 0; i < 3; ++i) {
            std::cout << m_menu_info[i] << '\n';
        }

        std::cout << "> "
                  << m_input_buffer
                  << std::flush;

        return 0;
    }

    /**
     * @brief Recebe mensagens do servidor para o agente trainer.
     *
     * Bloqueia até receber uma mensagem válida, depois drena o buffer
     * de mensagens pendentes (non-blocking) antes de retornar.
     * Não há processamento de mensagens.
     *
     * @return int 0 em sucesso, 1 se conexão for encerrada.
     */
    int perception_and_update_trainer() {

        std::string_view message_from_server {};
        while(true) {

            if(m_sc.isclosed()) {
                return 1;
            }
            message_from_server = m_sc.receive(false);
            if(message_from_server.empty()) {
                continue;
            }
            break;
        }
        if(message_from_server[1] != 't') { // Retiramos `(think)`
            bool is_see = message_from_server[1] == 's' &&
                          message_from_server[2] == 'e' &&
                          message_from_server[3] == 'e';

            m_menu_info[!is_see + 1] = message_from_server;
        }

        while(true) {

            if(m_sc.isclosed()) {
                return 1;
            }
            // Tenta receber dados sem bloquear
            message_from_server = m_sc.receive(true);

            if(message_from_server.empty()) {
                break;
            }
            if(message_from_server[1] != 't') { // Retiramos `(think)`
                bool is_see = message_from_server[1] == 's' &&
                              message_from_server[2] == 'e' &&
                              message_from_server[3] == 'e';

                m_menu_info[!is_see + 1] = message_from_server;
            }
        }
        return 0;
    }

    int run() {

        if(perception_and_update_trainer()) {
            return 1;
        }
        auto start_time = std::chrono::steady_clock::now();

        if(print_menu()){
            return 0;
        }
        check_and_send();

        auto end_time = std::chrono::steady_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        if(elapsed_time < m_target_duration) {
            std::this_thread::sleep_for(m_target_duration - elapsed_time);
        }
        m_sc.send_immediate("(done)");
        // Faremos aqui apenas para evitar atraso do `(done)`
        if(elapsed_time < m_target_duration) {
            m_menu_info[0] = std::format("Tempo Dormido: {}", m_target_duration - elapsed_time);
        }
        return 0;
    }
};