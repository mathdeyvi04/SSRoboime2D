#pragma once

#include <cstdint>
#include <string_view>
#include <optional>
#include <utility>
#include <array>
#include <charconv>
#include <iostream>

#include "../booting/Booting.hpp"

class Environment {
public:

    enum class PlayMode : uint8_t {
        // Neutros
        BEFORE_KICK_OFF = 0b0000'0000,
        PLAY_ON         = 0b0000'0001,

        // Esquerda

        // Direita
    };
    // ----- Atributos Gerais Comuns a Cada Jogador
    inline static bool is_left = false;
    inline static int cycle    = 0;

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

    // ----- Atributos Únicos a Cada Jogador
    /** @brief ID do jogador atribuído pelo servidor. */
    uint8_t unum = 0;

    /** @brief Configuração de visão. [0]: Qualidade (High/Low), [1]: Largura (Narrow/Normal/Wide). */
    std::array<uint8_t, 2> view_mode;

    /** @brief Gestão de energia. [0]: Stamina, [1]: Effort, [2]: Capacity. */
    std::array<size_t, 3> stamina_info;

    /** @brief Vetor velocidade relativo ao campo. [0]: vx, [1]: vy. */
    std::array<int, 2> speed = {0, 0};

    /** @brief Ângulo do pescoço relativo ao torso. Persiste após comando 'turn'. */
    int head_angle = 0;

    /** @brief Point-to. [0]: Movable, [1]: Expires, [2,3]: Target(X,Y). */
    std::array<int, 4> arm;

    /** @brief Foco sensorial, habilidade do jogador. [0]: Tipo, [1,2]: Meta(ID/XY), [3,4]: Pos(XY). */
    std::array<int, 5> focus;

    /** @brief Status de faltas. [0]: Ativo, [1]: Cartão tomado. */
    std::array<uint8_t, 2> fouls;

    class WorldParser {
    private:

        const char* cursor = nullptr;
        const char* end    = nullptr;

        /**
         * @brief Avança o cursor até encontrar o caractere especificado.
         *
         * @param caract Caractere alvo da busca.
         * @return true  Se o caractere foi encontrado.
         * @return false Se o fim do buffer foi atingido (cursor > end).
         *
         * @note O cursor avança incluindo o caractere encontrado.
         */
        bool skip_until_char(char caract) {
            while(*(this->cursor++) != caract) {
                if(this->cursor > this->end) {
                    return false;
                }
            }

            return true;
        }

        /**
         * @brief Extrai a próxima string delimitada por espaço ou parênteses.
         *
         * @return std::string_view Visão da string extraída (sem cópia).
         *
         * @details
         * - Pula espaços e '(' iniciais
         * - Lê até encontrar ' ' ou ')'
         * - Retorna a string entre os delimitadores
         */
        std::string_view get_next_str() {
            while(*this->cursor == ' ' || *this->cursor == '('){ this->cursor++; }
            const char* str_start = this->cursor;
            while(*this->cursor != ' ' && *this->cursor != ')'){ this->cursor++; }
            return {str_start, static_cast<size_t>(this->cursor - str_start)};
        }

        /**
         * @brief Avança o cursor ignorando blocos aninhados "()" desconhecidos.
         * @param init_count Número inicial de pares a ignorar (padrão=1).
         */
        void skip_unknown(uint8_t init_count = 1) {
            // Vamos considerar que acabamos de encontrar uma tag desconhecida, ou seja passamos por '('.
            // Devemos encontrar outro ')' para eliminar este e, finalmente, encerrar a função.

            uint8_t count_pair = init_count;
            while(count_pair != 0) {
                count_pair += (*this->cursor == '(') * (1) + (*this->cursor == ')') * (-1);
                this->cursor++;
            }
        }

        /**
         * @brief Interpreta mensagem "(sense_body)" do servidor.
         *
         * @param env Ambiente onde os dados serão armazenados.
         *
         * @details Processa:
         * - view_mode
         * - stamina, speed, head_angle
         * - arm
         * - focus
         * - foul
         * - focus_point
         */
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
                        /* Como são informações inúteis, vamos apenas
                           pulá-los                                */
                        float total_actioncounters = 9.0;
                        while(total_actioncounters) {
                            if(*this->cursor == '(' || *this->cursor == ')') {
                                total_actioncounters -= 0.5;
                            }
                            this->cursor++;
                        }
                        break;
                    }
                    case 'a': { // arm
                        for(uint8_t i = 0; i < 3; ++i) {
                            std::string_view str_value = this->get_next_str();
                            if(str_value[0] == 't') { // target
                                str_value = this->get_next_str();
                                std::from_chars(
                                    str_value.data(),
                                    str_value.data() + str_value.size(),
                                    env.arm[i]
                                );
                                str_value = this->get_next_str();
                                std::from_chars(
                                    str_value.data(),
                                    str_value.data() + str_value.size(),
                                    env.arm[++i]
                                );
                            }
                            else { // movable expires
                                str_value = this->get_next_str();
                                std::from_chars(
                                    str_value.data(),
                                    str_value.data() + str_value.size(),
                                    env.arm[i]
                                );
                            }
                            this->cursor++;
                        }
                        // Devemos pular o count, que é inútil
                        this->skip_unknown();
                        break;
                    }
                    case 'f': {
                        switch(lower_tag.size()) {
                            case 5: { // focus
                                // Pulamos o próximo nome, pois sabemos que é target
                                this->get_next_str();
                                // Vamos ao objeto de foco
                                std::string_view str_value = this->get_next_str();
                                switch(str_value[2]) {
                                    case 'n': { // none
                                        this->cursor++;
                                        break;
                                    }

                                    case 'l': { // ball
                                        env.focus[0] = 1;
                                        this->cursor++;
                                        break;
                                    }

                                    case 'a': { // player

                                        env.focus[0] = 2;
                                        this->cursor++;
                                        env.focus[1] = (
                                            (*this->cursor == 'r') && Environment::is_left
                                        ) ? 1 : -1;
                                        this->cursor++;
                                        str_value = this->get_next_str();
                                        std::from_chars(
                                            str_value.data(),
                                            str_value.data() + str_value.size(),
                                            env.focus[2]
                                        );
                                        this->cursor += 2;
                                        break;
                                    }

                                    case 'i': { // point

                                        env.focus[0] = 3;
                                        std::string_view str_value = this->get_next_str();
                                        std::from_chars(
                                            str_value.data(),
                                            str_value.data() + str_value.size(),
                                            env.focus[1]
                                        );
                                        str_value = this->get_next_str();
                                        std::from_chars(
                                            str_value.data(),
                                            str_value.data() + str_value.size(),
                                            env.focus[2]
                                        );
                                        this->cursor += 2;
                                        break;
                                    }
                                }

                                // Devemos pular o count, pois é inútil
                                this->skip_unknown();

                                // Devemos pular o tackle, pois não o julguei importante o suficiente
                                this->cursor += 3;
                                this->skip_unknown();

                                // Devemos pular o collision, pois não o julguei importante
                                this->cursor += 3;
                                this->skip_unknown();
                                break;
                            }
                            case 4: { // foul
                                // charged
                                this->get_next_str();
                                this->cursor++;
                                env.fouls[0] = *(this->cursor++) - 48;
                                this->cursor++;
                                // card
                                this->get_next_str();
                                std::string_view str_value = this->get_next_str();
                                switch(str_value[0]) {
                                    case 'n': {
                                        env.fouls[1] = 0;
                                        break;
                                    }
                                    case 'y': {
                                        env.fouls[1] = 1;
                                        break;
                                    }
                                    case 'r': {
                                        env.fouls[1] = 2;
                                        break;
                                    }
                                }
                                this->cursor += 3;
                                break;
                            }

                            case 11: { // focus_point
                                std::string_view str_value = this->get_next_str();
                                std::from_chars(
                                    str_value.data(),
                                    str_value.data() + str_value.size(),
                                    env.focus[3]
                                );
                                str_value = this->get_next_str();
                                std::from_chars(
                                    str_value.data(),
                                    str_value.data() + str_value.size(),
                                    env.focus[4]
                                );
                                this->cursor++; // Vai restar ')', o que ativará o trigger do tamanho 0 no default
                                break;
                            }
                        }
                        break;
                    }
                    default: {
                        if(lower_tag.size() == 0) {
                            return;
                        }
                        break;
                    }
                }
            }
        }

        void parse_see(Environment& env) {

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

                                this->parse_see(env);
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