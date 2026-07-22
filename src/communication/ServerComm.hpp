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

class ServerComm {
public:
    /** @brief File Descriptor do Socket */
    int m_fd {};
    /** @brief Endereço e Porta do Server */
    sockaddr_in m_serveraddr {};
    /** @brief Para tamanho do buffer de mensagens do servidor */
    /** @brief Sei que costuma ser menos, mas as primeiras explodem e isso pode causar confusão */
    inline static constexpr int SIZEBUFFER {4096};
    /** @brief Tempo de TIMEOUT dos sockets de comunicação */
    inline static constexpr int TIMEOUTSOCKETSERVER {200000}; // Em milisegundos
    /** @brief Buffer para mensagens do Server */
    std::array<char, ServerComm::SIZEBUFFER> m_buffer {};
    /** @brief Inteiro para verificarmos se houve desconexão */
    size_t m_disconnect {};
    /** @brief Flag para identificarmos se é Trainer Agent */
    bool m_is_trainer_agent {false};
    /** @brief Precisamos do número do jogador a fim de posicioná-lo corretamente no início */
    uint8_t m_unum {};
    /** @brief Contador de Conexões ao Servidor, será atomic inclusive para o single-thread */
    inline static std::atomic<uint8_t> NUMBER_PLAYERS {};

    /**
     * @brief Construtor que inicializa conexão UDP com o servidor RCSS.
     *
     * Estabelece socket, realiza handshake com mensagem INIT e configura timeout.
     * @param team_name Nome do Time
     * @param ip Endereço IP do servidor.
     * @param port Porta do servidor.
     * @param is_trainer_agent Se true, conecta como técnico (sem uniforme).
     */
    ServerComm(const std::string& team_name, const std::string& ip, int port, bool is_trainer_agent = false) {

        m_is_trainer_agent = is_trainer_agent;
        if(!m_is_trainer_agent) {
            m_unum = ++ServerComm::NUMBER_PLAYERS;
        }

        // Definições do Socket e da Comunicação
        m_fd = socket(AF_INET, SOCK_DGRAM, 0);
        m_serveraddr.sin_family = AF_INET;
        m_serveraddr.sin_port = htons(port);
        inet_pton(
            AF_INET,
            ip.c_str(),
            &m_serveraddr.sin_addr
        );

        // Handshake
        std::string init_msg;
        if(!m_is_trainer_agent) {
            if(ServerComm::NUMBER_PLAYERS == 11) {
                init_msg = std::format("(init {} (version 18) (goalie))", team_name);
            }
            else {
                init_msg = std::format("(init {} (version 18))", team_name);
            }
        }
        else{
            init_msg = "(init (version 18))";
        }
        sendto(
            m_fd,
            init_msg.c_str(),
            init_msg.size(),
            0,
            reinterpret_cast<sockaddr*>(&m_serveraddr),
            sizeof(m_serveraddr)
        );
        sockaddr_in from;
        socklen_t fromlength = sizeof(from);
        recvfrom(
            m_fd,
            m_buffer.data(),
            m_buffer.size(),
            0,
            // Kernel escreverá nesses endereços
            reinterpret_cast<sockaddr*>(&from),
            &fromlength
        );
        // Troca de Portas do Server
        m_serveraddr.sin_port = from.sin_port;
        connect(
            m_fd,
            reinterpret_cast<sockaddr*>(&m_serveraddr),
            sizeof(m_serveraddr)
        );
        // Define timeout para operações de recebimento (bloqueia até timeout)
        struct timeval tv;
        tv.tv_sec  = 0;
        tv.tv_usec = ServerComm::TIMEOUTSOCKETSERVER;
        setsockopt(
            m_fd,
            SOL_SOCKET,
            SO_RCVTIMEO,
            &tv,
            sizeof(tv)
        );

        /* Caso seja um TrainerAgent, precisamos enviar mais um comando */
        if(m_is_trainer_agent) {
            send_immediate("(eye on)");
        }
    }

    /**
     * @brief Encerra a conexão com o servidor.
     *
     * Envia comando de despedida "(bye)", fecha o socket e
     * redefine o descritor do socket para 0.
     */
    void termined() {
        send_immediate("(bye)");
        close(m_fd);
        m_fd = 0;
    }

    /**
     * @brief Verifica se a conexão está encerrada.
     *
     * @return true  Se o socket está fechado (m_fd == 0)
     * @return false Se o socket ainda está aberto (m_fd != 0)
     */
    bool isclosed() {
        return m_fd == 0;
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
            m_fd,
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
            m_fd,
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
        if (isclosed()) {
            return {};
        }

        // Aguarda dados bloqueantemente
        int n = recv(
            m_fd,
            m_buffer.data(),
            m_buffer.size(),
            // Se non_blocking for true, usamos MSG_DONTWAIT para não travar caso o buffer esteja vazio
            non_blocking ? MSG_DONTWAIT : 0
        );

        if (n > 0) {
            // Dados recebidos com sucesso: reseta contador de desconexão
            m_disconnect = 0;
            return {m_buffer.data(), static_cast<size_t>(n)};
        }
        else if (n == -1) {
            // Se for MSG_DONTWAIT e não tiver mais dados, errno será EAGAIN/EWOULDBLOCK
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                if (!non_blocking) {
                    // Só conta como falha/timeout real se era para ter bloqueado
                    if (++m_disconnect >= 3) {
                        termined();
                    }
                }
                return {};
            }
            else {
                // Outro erro (conexão resetada, socket inválido)
                termined();
                return {};
            }
        }
        else {
            // n == 0 (servidor fechou conexão, raro em UDP, mas por segurança)
            termined();
            return {};
        }
    }
};