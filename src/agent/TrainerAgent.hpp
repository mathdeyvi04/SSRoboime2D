#pragma once

#include <iostream>

#include "../communication/ServerComm.hpp"

class TrainerAgent {
private:

    /* Forma de Comunicação com Servidor */
    ServerComm __sc;
public:

    TrainerAgent(
        const std::string& ip,
        int port,
        bool verbose
    ) :
        __sc{ip, port, true}
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

            if(this->__sc.isclosed()) {
//                this->__env.logger.info(
//                    std::format(
//                        "Jogador {} saiu de campo.",
//                        this->__env.unum
//                    )
//                );
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
//        this->__env.wp.update_from_server(
//            message_from_server,
//            this->__env
//        );

        // Loop não-bloqueante: drena o buffer do socket
        // Processa todas as mensagens pendentes
        while(true) {

            if(this->__sc.isclosed()) {
//                this->__env.logger.info(
//                    std::format(
//                        "Jogador {} saiu de campo.",
//                        this->__env.unum
//                    )
//                );
                return 1;
            }
            // Tenta receber dados sem bloquear
            message_from_server = this->__sc.receive(true);
            if(message_from_server.empty()) {
                break;  // Buffer do SO está vazio, não há mais dados
            }
            // Processa cada mensagem adicional recebida
//            this->__env.wp.update_from_server(
//                message_from_server,
//                this->__env
//            );
        }
        return 0;
    }

    int run() {

        if(this->perception_and_update()) {
            return 1;
        }

        return 0;
    }
};