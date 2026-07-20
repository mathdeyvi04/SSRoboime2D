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
#include <atomic>

#include "../booting/Booting.hpp"

class ServerComm {
private:
    // File Descriptor do Socket
    int __fd {};
    // Endereço e Porta do Server
    sockaddr_in __serveraddr {};
    // Buffer para mensagens do Server
    std::array<char, Booting::SIZEBUFFER> __buffer {};
    // Inteiro para verificarmos se houve desconexão
    size_t __disconnect {};
    // Flag para identificarmos se é Trainer Agent
    bool is_trainer_agent {false};

public:

    // Precisamos do número do jogador a fim de posicioná-lo corretamente no início
    uint8_t unum {};

    /** @brief Contador de Conexões ao Servidor, será atomic inclusive para o single-thread */
    inline static std::atomic<uint8_t> number_players {};

    /**
     * @brief Construtor que inicializa conexão UDP com o servidor RCSS.
     *
     * Estabelece socket, realiza handshake com mensagem INIT e configura timeout.
     *
     * @param ip Endereço IP do servidor.
     * @param port Porta do servidor.
     * @param is_trainer_agent Se true, conecta como técnico (sem uniforme).
     */
    ServerComm(const std::string& ip, int port, bool is_trainer_agent = false) {

        this->is_trainer_agent = is_trainer_agent;
        if(!is_trainer_agent) {
            this->unum = ++ServerComm::number_players;
        }

        // Definições do Socket e da Comunicação
        this->__fd = socket(AF_INET, SOCK_DGRAM, 0);
        this->__serveraddr.sin_family = AF_INET;
        this->__serveraddr.sin_port = htons(port);
        inet_pton(
            AF_INET,
            ip.c_str(),
            &this->__serveraddr.sin_addr
        );

        // Handshake
        std::string init_msg;
        if(!is_trainer_agent) {
            if(ServerComm::number_players == 11) {
                init_msg = std::format("(init {} (version 18) (goalie))", Booting::TEAMNAME);
            }
            else {
                init_msg = std::format("(init {} (version 18))", Booting::TEAMNAME);
            }
        }
        else{
            init_msg = "(init (version 18))";
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
        tv.tv_sec  = 0;
        tv.tv_usec = 200000;
        setsockopt(
            this->__fd,
            SOL_SOCKET,
            SO_RCVTIMEO,
            &tv,
            sizeof(tv)
        );

        /* Caso seja um TrainerAgent, precisamos enviar mais um comando */
        if(is_trainer_agent) {
            this->send_immediate("(eye on)");
        }
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
     * @brief Envia mensagem imediatamente a partir de uma string.
     * @details Incrementamos o size para forçar o envio do caractere nulo.
     * @param msg Mensagem a ser enviada.
     * @return true Se pelo menos um byte foi enviado.
     * @return false Se nenhum byte foi enviado (erro ou conexão fechada).
     */
    bool send_immediate(const std::string& msg) {
        return send(
            this->__fd,
            msg.data(),
            msg.size() + 1,
            0
        ) > 0;
    }

    /**
     * @brief Envia dados binários ou texto diretamente do buffer.
     * @details Incrementamos o size para forçar o envio do caractere nulo.
     * @param data Ponteiro para os dados a serem enviados (geralmente buffer.data()).
     * @param size Número de bytes a serem transmitidos.
     * @return true Se pelo menos um byte foi enviado.
     * @return false Se nenhum byte foi enviado (erro ou conexão fechada).
     */
    bool send_immediate(
        const char* data,
        size_t size
    ) {
        return send(
            this->__fd,
            data,
            size + 1,
            0
        ) > 0;
    }

    /**
     * @brief Recebe dados do servidor via socket UDP.
     * @param non_blocking Se true, retorna imediatamente se não houver dados disponíveis.
     * @return std::string_view Dados recebidos (vazio se timeout, erro ou sem dados).
     * @note Após 3 timeouts consecutivos em modo bloqueante, a conexão é encerrada.
     */
    std::string_view receive(
        bool non_blocking = false
    ) {
        // Proteção contra uso após fechamento
        if (this->isclosed()) {
            return {};
        }

        // Aguarda dados bloqueantemente
        int n = recv(
            this->__fd,
            this->__buffer.data(),
            this->__buffer.size(),
            // Se non_blocking for true, usamos MSG_DONTWAIT para não travar caso o buffer esteja vazio
            non_blocking ? MSG_DONTWAIT : 0
        );

        if (n > 0) {
            // Dados recebidos com sucesso: reseta contador de desconexão
            this->__disconnect = 0;
            return {this->__buffer.data(), static_cast<size_t>(n)};
        }
        else if (n == -1) {
            // Se for MSG_DONTWAIT e não tiver mais dados, errno será EAGAIN/EWOULDBLOCK
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                if (!non_blocking) {
                    // Só conta como falha/timeout real se era para ter bloqueado
                    if (++this->__disconnect >= 3) {
                        this->termined();
                    }
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
