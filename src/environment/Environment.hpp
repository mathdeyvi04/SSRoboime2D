#pragma once

#include <cstdint>
#include <string_view>
#include <functional>
#include <optional>
#include <utility>
#include <array>
#include <iostream>

class Environment {
public:

    // Atributos de Partida
    inline static bool is_left = false;

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
    PlayMode pm;




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
            while(*this->cursor == ' '){ this->cursor++; }
            const char* str_start = this->cursor;
            while(*this->cursor != ' ' && *this->cursor != ')'){ this->cursor++; }
            return {str_start, static_cast<size_t>(this->cursor - str_start)};
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
                        env.is_left = this->get_next_str()[0] == 'l';
                        // Devemos pular o número de uniforme, pois já está salvo no ServerComm
                        this->get_next_str();
                        // É garantido que teremos is_left definido daqui em diante
                        std::string_view possible_mode = this->get_next_str();
                        std::optional<Environment::PlayMode> result_from_search = Environment::get_play_mode(possible_mode);
                        if(result_from_search.has_value()) {
                            env.pm = std::move(result_from_search.value());
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

                                break;
                            }

                            case 12: { // server_param

                                break;
                            }
                        }

                        break;
                    }

                    case 'p': { // player_param player_type

                        break;
                    }

                    case 'o': { // ok

                        break;
                    }

                    default:
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