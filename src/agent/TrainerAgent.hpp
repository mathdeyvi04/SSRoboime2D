#pragma once

#include <iostream>

#include "../communication/ServerComm.hpp"

class TrainerAgent {
private:

    /* Forma de Comunicação com Servidor */
    ServerComm m_sc;



public:

    TrainerAgent(
        const std::string& ip,
        int port,
        bool verbose
    ) :
        m_sc{"", ip, port, true}
    {}

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

            if(m_sc.isclosed()) {
                return 1;
            }
            // Tenta receber dados do servidor (modo bloqueante)
            message_from_server = m_sc.receive(false);
            if(message_from_server.empty()) {
                continue;  // Timeout temporário, tenta novamente
            }
            break;  // Mensagem recebida com sucesso
        }
        std::cout << message_from_server << std::endl;

        // Loop não-bloqueante: drena o buffer do socket
        // Processa todas as mensagens pendentes
        while(true) {

            if(m_sc.isclosed()) {
                return 1;
            }
            // Tenta receber dados sem bloquear
            message_from_server = m_sc.receive(true);

            if(message_from_server.empty()) {
                break;  // Buffer do SO está vazio, não há mais dados
            }
            // Processa cada mensagem adicional recebida
            std::cout << message_from_server << std::endl;
        }
        return 0;
    }

    int run() {

        if(perception_and_update()) {
            return 1;
        }

        /*
        O que você deseja aqui!

        Estilo de mensagens que recebe:
        (think)
        (see_global 152 ((g r) 52.5 0) ((g l) -52.5 0) ((b) 0 0 0 0))
        */

        m_sc.send_immediate("(done)");
        return 0;
    }
};