#pragma once

#include <iostream>
#include <chrono>
#include <thread>
#include "../communication/ServerComm.hpp"

class TrainerAgent {
private:

    /** @brief Forma de Comunicação com Servidor */
    ServerComm m_sc;

    /** @brief Temporizador entre ciclos, terá como base o valor de 100 ms */
    std::chrono::milliseconds m_target_duration;

public:

    TrainerAgent(
        const std::string& ip,
        int port,
        float speed
    ) :
        m_sc{"", ip, port, true},
        m_target_duration{static_cast<const unsigned long>(speed * 100)}
    {}

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
//        std::cout << message_from_server << std::endl;

        while(true) {

            if(m_sc.isclosed()) {
                return 1;
            }
            // Tenta receber dados sem bloquear
            message_from_server = m_sc.receive(true);

            if(message_from_server.empty()) {
                break;
            }
//            std::cout << message_from_server << std::endl;
        }
        return 0;
    }

    int run() {

        if(perception_and_update_trainer()) {
            return 1;
        }
        auto start_time = std::chrono::steady_clock::now();

        /* Aqui colocaremos o sanha do treinamento */



        auto end_time = std::chrono::steady_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        if(elapsed_time < m_target_duration) {
            std::this_thread::sleep_for(m_target_duration - elapsed_time);
        }
        m_sc.send_immediate("(done)");
        return 0;
    }
};