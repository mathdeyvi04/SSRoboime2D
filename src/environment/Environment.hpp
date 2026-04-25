#pragma once

#include <cstdint>
#include <string_view>
#include <optional>
#include <utility>
#include <array>
#include <charconv>
#include <iostream>

class Environment {
public:

    // Atributos Gerais Comuns a Cada Jogador
    inline static bool is_left = false;
    inline static int cycle    = 0;
    enum class PlayMode : uint8_t {
        // Neutros
        BEFORE_KICK_OFF = 0b0000'0000,
        PLAY_ON         = 0b0000'0001,

        // Esquerda

        // Direita

    };
    inline static const std::optional<Environment::PlayMode> get_play_mode(const std::string_view& key) {
        static constexpr std::array <
            std::pair<std::string_view, std::array<Environment::PlayMode, 2>>, 2
        > dict_play_modes = {{
            {"before_kick_off", {PlayMode::BEFORE_KICK_OFF, PlayMode::BEFORE_KICK_OFF}},
            {"play_on", {PlayMode::PLAY_ON, PlayMode::PLAY_ON}}
        }};

        for(const auto& elemento : dict_play_modes) {
            if(elemento.first == key) {
                return elemento.second[Environment::is_left];
            }
        }

        return std::nullopt;
    }
    inline static PlayMode pm;

    // Atributos Únicos a Cada Jogador
    /* Atualizaremos esse valor a partir do ServerComm */
    uint8_t unum = 0;
    /*
    [0] -> qualidade [1] -> largura
    Possíveis valores de qualidade: (0, high) e (1, low)
    Possíveis valores de largura: (0, narrow), (1, normal), (2, wide)
    Cada combinação possível uma taxa de envio de mensagens específicas
    */
    std::array<uint8_t, 2> view_mode;
    /*
    [stamina atual, effort atual, reserva total de stamin]
    */
    std::array<size_t, 3> stamina_info;
    std::array<int, 2> speed = {0, 0};
    int head_angle = 0;
    std::array<uint8_t, 9> action_counters;

    class WorldParser {
    private:

        const char* cursor = nullptr;
        const char* end    = nullptr;

        bool skip_until_char(char caract) {
            while(*(this->cursor++) != '(') {
                if(this->cursor > this->end) {
                    return false;
                }
            }

            return true;
        }

        std::string_view get_next_str() {
            while(*this->cursor == ' ' || *this->cursor == '('){ this->cursor++; }
            const char* str_start = this->cursor;
            while(*this->cursor != ' ' && *this->cursor != ')'){ this->cursor++; }
            return {str_start, static_cast<size_t>(this->cursor - str_start)};
        }

        void skip_unknown() {
            // Vamos considerar que acabamos de encontrar uma tag desconhecida, ou seja passamos por '('.
            // Devemos encontrar outro ')' para eliminar este e, finalmente, encerrar a função.

            uint8_t count_pair = 1;
            while(count_pair != 0) {
                count_pair += (*this->cursor == '(') * (1) + (*this->cursor == ')') * (-1);
                this->cursor++;
            }
        }

        void parse_sensebody(Environment& env) {

            // Por algum motivo, o primeiro valor está separado.
            std::string_view str_cycle = this->get_next_str();
            if(env.unum == 1) {
                std::from_chars(str_cycle.data(), str_cycle.data() + str_cycle.size(), Environment::cycle);
            }

            while(true) {

                std::string_view lower_tag = this->get_next_str();

                switch(lower_tag[0]) {

                    case 'v': { // view_mode

                        this->cursor += 1;
                        switch(*this->cursor) {

                            case 'h': {

                                env.view_mode[0] = 0;
                                this->cursor += 4;
                                break;
                            }

                            case 'l': {

                                env.view_mode[0] = 1;
                                this->cursor += 3;
                                break;
                            }
                        }

                        this->cursor += 2;
                        switch(*this->cursor) {

                            case 'a': { // narrow
                                env.view_mode[1] = 0;
                                break;
                            }

                            case 'o': { // normal
                                env.view_mode[1] = 1;
                                break;
                            }

                            case 'i': { // wide
                                env.view_mode[1] = 2;
                                break;
                            }
                        }

                        this->skip_until_char(')');
                        break;
                    }

                    case 's': { // stamina speed

                        if(lower_tag[1] == 't') {
                            for(auto& elemento : env.stamina_info) {
                                std::string_view str_value = this->get_next_str();
                                std::from_chars(
                                    str_value.data(),
                                    str_value.data() + str_value.size(),
                                    elemento
                                );
                            }
                        }
                        else if(lower_tag[1] == 'p') {
                            for(auto& elemento : env.speed) {
                                std::string_view str_value = this->get_next_str();
                                std::from_chars(
                                    str_value.data(),
                                    str_value.data() + str_value.size(),
                                    elemento
                                );
                            }
                        }

                        this->skip_until_char(')');
                        break;
                    }

                    case 'h': { // head_angle
                        std::string_view str_head_angle = this->get_next_str();
                        std::from_chars(
                            str_head_angle.data(),
                            str_head_angle.data() + str_head_angle.size(),
                            env.head_angle
                        );
                        this->cursor++;

                        /* Sabemos que agora vem os ActionCounters */
                        /* Vamos tratá-los aqui de uma vez         */
                        for(auto& action_counter : env.action_counters) {
                            this->get_next_str(); // jogamos o nome no lixo, pois já sabemos a ordem
                            std::string_view str_value = this->get_next_str();
                            std::from_chars(
                                str_value.data(),
                                str_value.data() + str_value.size(),
                                action_counter
                            );
                        }

                        break;
                    }

                    case 'a': {



                        break;
                    }

                    default: {
                        break;
                    }
                }
            }




        }

    public:

        void update_from_server(
            const std::string_view& message_from_server,
            Environment& env
        ) {
            // Definimos atributos cruciais
            this->cursor = message_from_server.data();
            this->end    = message_from_server.data() + message_from_server.size();

            while(this->cursor < this->end) {

                if(!this->skip_until_char('(')) {
                    return;
                }

                std::string_view uppest_tag = this->get_next_str();
                switch (uppest_tag[0]) {

                    case 'i': { // init

                        if(env.unum != 1){
                            // Para que seja thread-safe, permitiremos que apenas o jogador 1 faça essas alterações.
                            break;
                        }
                        Environment::is_left = this->get_next_str()[0] == 'l';
                        // Devemos pular o número de uniforme, pois já está salvo no ServerComm
                        this->get_next_str();
                        // É garantido que teremos is_left definido daqui em diante
                        std::string_view possible_mode = this->get_next_str();
                        std::optional<Environment::PlayMode> result_from_search = Environment::get_play_mode(possible_mode);
                        if(result_from_search.has_value()) {
                            Environment::pm = std::move(result_from_search.value());
                        }
                        else {
                            std::cout << "Modo Desconhecido: " << possible_mode << std::endl;
                        }

                        break;
                    }

                    case 's': { // server_param see sense_body

                        switch (uppest_tag.size()) {

                            case 3: { // see

                                break;
                            }

                            case 10: { // sense_body

                                this->parse_sensebody(env);
                                break;
                            }

                            case 12: { // server_param

                                this->skip_unknown();
                                break;
                            }
                        }

                        break;
                    }

                    case 'p': { // player_param player_type

                        this->skip_unknown();
                        break;
                    }

                    case 'o': { // ok

                        this->skip_unknown();
                        break;
                    }

                    default:
                        this->skip_unknown();
                        std::cout << "Tag Desconhecida: " << uppest_tag << std::endl;
                }

            }

            // Retornarmos seus valores ao nulo para evitar qualquer descuido
            this->cursor = nullptr;
            this->end    = nullptr;
        }
    };
    WorldParser wp;
};