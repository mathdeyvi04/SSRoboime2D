#pragma once

#include <variant>
#include <memory>
#include <cstring> // memcpy
#include <charconv>
#include <array>

namespace BasicCommands {
    struct Dash {
        double power {};  // potência do dash (positiva para frente, negativa para trás)

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
        double angle_body {};  // ângulo de rotação (em graus, positivo para anti-horário)

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
        double angle_head {};  // ângulo de rotação do pescoço (visão)

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
        double x {};  // coordenada X (campo, absoluta)
        double y {};  // coordenada Y

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
        std::string message {};  // texto da mensagem (sem espaços, conforme protocolo)

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
        double power {};
        double direction {};

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
        double power {};     // força do tackle
        double direction {}; // direção (ângulo) do tackle

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
        double direction {};  // ângulo para onde estender as mãos (para pegar a bola)

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
        Width width {};

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
}