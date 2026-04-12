#pragma once

// --- Para manipulação eficiente de strings e de arrays
#include <format>
#include <string>
#include <string_view>
#include <array>

// --- Para a manipulação de sockets em ambiente Linux
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include "../Booting.hpp"

class ServerComm {
private:
    // File Descriptor do Socket
    int __fd;
    // Endereço e Porta do Server
    sockaddr_in __serveraddr;
    // Buffer para mensagens do Server
    std::array<char, Booting::SIZEBUFFER> __buffer;

public:

    /**
     * @brief Construtor: conecta ao servidor e realiza handshake UDP.
     * @details Handshake (troca de porta):
     *          1. Envia "(init NAME (version 18))" para porta fixa (Booting::PORTSERVER)
     *          2. Recebe resposta, capturando a porta real do servidor (from.sin_port)
     *          3. Reconfigura destino para a nova porta
     *          4. Aplica connect() para envio/recebimento simplificado
     *          5. Configura socket como não-bloqueante
     */
    ServerComm() {
        // Definições do Socket e da Comunicação
        this->__fd = socket(AF_INET, SOCK_DGRAM, 0);
        this->__serveraddr.sin_family = AF_INET;
        this->__serveraddr.sin_port = htons(Booting::PORTSERVER);
        inet_pton(
            AF_INET,
            Booting::IPSERVER,
            &this->__serveraddr.sin_addr
        );

        // Handshake
        std::string init_msg = std::format(
            "(init {} (version 18))",
            Booting::TEAMNAME
        );
        sendto(
            this->__fd,
            init_msg.c_str(),
            init_msg.size(),
            0,
            reinterpret_cast<sockaddr*>(&this->__serveraddr),
            sizeof(this->__serveraddr)
        );
        sockaddr_in from;
        socklen_t fromlength = sizeof(from);
        recvfrom(
            this->__fd,
            this->__buffer.data(),
            this->__buffer.size(),
            0,
            // Kernel escreverá nesses endereços
            reinterpret_cast<sockaddr*>(&from),
            &fromlength
        );
        // Troca de Portas do Server
        this->__serveraddr.sin_port = from.sin_port;
        connect(
            this->__fd,
            reinterpret_cast<sockaddr*>(&this->__serveraddr),
            sizeof(this->__serveraddr)
        );
        // Definimos como não bloqueante
        fcntl(this->__fd, F_SETFL, O_NONBLOCK);
    }

    ~ServerComm(){ close(this->__fd); }

    /**
     * @brief Envia mensagem imediatamente
     * @param msg Mensagem a ser enviada.
     * @return Se enviou alguma quantidade de bytes
     */
    bool send_immediate(const std::string& msg) {
        return send(
            this->__fd,
            msg.data(),
            msg.size(),
            0
        ) > 0;
    }

    /**
     * @brief Verifica se há dados disponíveis para leitura no socket.
     * @details Acontece que a função `select` escreve nos endereços fornecidos
     * modificando-os, por isso temos que declará-los a cada uso na função
     * @return 1 Se há dados (pronto para ler)
     * @return 0 Se timeout expirou
     * @return -1 Se erro no select
     */
    int is_readable() {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(this->__fd, &read_fds);
        struct timeval timeout = {
            .tv_sec = 0,
            .tv_usec = Booting::TIMEOUTSOCKETSERVER * 1000
        };

        return select(
            this->__fd + 1,
            &read_fds,
            nullptr,
            nullptr,
            &timeout
        ) > 0;
    }

    /**
     * @brief Tenta receber dados do socket (não-bloqueante).
     * @return std::string_view Dados recebidos (vazio se timeout/erro).
     * @note Verifica disponibilidade com is_readable() antes de recv().
     */
    std::string_view receive() {
        if(this->is_readable() > 0){
            ssize_t n = recv(this->__fd, this->__buffer.data(), this->__buffer.size(), 0);
            return {this->__buffer.data(), static_cast<size_t>(n)};
        }

        return {};
    }
};
