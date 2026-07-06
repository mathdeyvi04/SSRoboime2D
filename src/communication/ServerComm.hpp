#pragma once

// --- Para manipulação eficiente de strings e de arrays
#include <format>
#include <string>
#include <string_view>
#include <cstdint>
#include <array>

// --- Para a manipulação de sockets em ambiente Linux
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>

#ifdef MULTITHREAD
#include <atomic>
#endif

#include "../booting/Booting.hpp"

class ServerComm {
private:
    // File Descriptor do Socket
    int __fd;
    // Endereço e Porta do Server
    sockaddr_in __serveraddr;
    // Buffer para mensagens do Server
    std::array<char, Booting::SIZEBUFFER> __buffer;
    // Inteiro para verificarmos se houve desconexão
    size_t __disconnect = 0;

public:

    // Precisamos do número do jogador a fim de posicioná-lo corretamente no início
    uint8_t unum = 0;

#ifdef MULTITHREAD
    inline static std::atomic<uint8_t> number_players = 0;
#else
    inline static uint8_t number_players = 0;
#endif

    /**
     * @brief Construtor: conecta ao servidor e realiza handshake UDP.
     * @param is_last_one Booleano que indica se o jogador é o último
     * @details Handshake (troca de porta):
     *          1. Envia "(init NAME (version 18))" para porta fixa (Booting::PORTSERVER)
     *          2. Recebe resposta, capturando a porta real do servidor (from.sin_port)
     *          3. Reconfigura destino para a nova porta
     *          4. Aplica connect() para envio/recebimento simplificado
     */
    ServerComm() {
        this->unum = ++ServerComm::number_players;

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
        std::string init_msg;
        if(ServerComm::number_players == 11) {
            init_msg = std::format("(init {} (version 18) (goalie))", Booting::TEAMNAME);
        }
        else {
            init_msg = std::format("(init {} (version 18))", Booting::TEAMNAME);
        }
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
        // Define timeout para operações de recebimento (bloqueia até timeout)
        struct timeval tv;
        tv.tv_sec  = 1; // -- Deixaremos esperando por 1 segundo no máximo
        tv.tv_usec = 0;
        setsockopt(
            this->__fd,
            SOL_SOCKET,
            SO_RCVTIMEO,
            &tv,
            sizeof(tv)
        );
    }

    /**
     * @brief Encerra a conexão com o servidor.
     *
     * Envia comando de despedida "(bye)", fecha o socket e
     * redefine o descritor do socket para 0.
     */
    void termined() {
        this->send_immediate("(bye)");
        close(this->__fd);
        this->__fd = 0;
    }

    /**
     * @brief Verifica se a conexão está encerrada.
     *
     * @return true  Se o socket está fechado (__fd == 0)
     * @return false Se o socket ainda está aberto (__fd != 0)
     */
    bool isclosed() {
        return this->__fd == 0;
    }

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
     * @brief Tenta receber dados do socket.
     * @return std::string_view Dados recebidos (vazio se timeout/erro).
     */
    std::string_view receive() {
        // Proteção contra uso após fechamento
        if(this->isclosed()) {
            return {};
        }

        // Aguarda dados bloqueantemente
        ssize_t n = recv(
            this->__fd,
            this->__buffer.data(),
            this->__buffer.size(),
            0
        );

        if(n > 0) {
            // Dados recebidos com sucesso: reseta contador de desconexão
            this->__disconnect = 0;
            return {this->__buffer.data(), static_cast<size_t>(n)};
        }
        else if(n == -1) {
            // Verifica se foi timeout (servidor não respondeu dentro do prazo)
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Timeout: incrementa contador de falhas consecutivas
                if (++this->__disconnect >= 3) {
                    // Após timeouts consecutivos, considera servidor morto
                    this->termined();
                }
                return {};
            }
            else {
                // Outro erro (conexão resetada, socket inválido)
                this->termined();
                return {};
            }
        }
        else {
            // n == 0 (servidor fechou conexão, raro em UDP, mas por segurança)
            this->termined();
            return {};
        }
    }
};
