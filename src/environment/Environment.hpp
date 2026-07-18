#pragma once

#include <cstdint>
#include <string_view>
#include <optional>
#include <utility>
#include <array>
#include <charconv>
#include <algorithm>
#include <span>

#include "../booting/Booting.hpp"
#include "../logger/Logger.hpp"
#include "../math/GeneralMath.hpp"

class Environment {
public:

    Logger& logger = Logger::get();
    enum class PlayMode : uint8_t {
        // Neutros
        BEFORE_KICK_OFF,
        PLAY_ON,

        // Nossos
        OUR_KICK_OFF,

        // Deles
        OPP_KICK_OFF,
    };
    // ----- Atributos Gerais Comuns a Cada Jogador
    // Apenas jogador 1 modifica essa variáveis, logo não há race conditions
    inline static bool is_left {false};
    inline static int cycle    {0};

    inline static std::optional<Environment::PlayMode> get_play_mode(const std::string_view& key) {
        static constexpr std::array <
            std::pair<std::string_view, std::array<Environment::PlayMode, 2>>, 4
        > dict_play_modes = {{
            {"before_kick_off", {PlayMode::BEFORE_KICK_OFF, PlayMode::BEFORE_KICK_OFF}},
            {"play_on", {PlayMode::PLAY_ON, PlayMode::PLAY_ON}},
            {"kick_off_l", {PlayMode::OUR_KICK_OFF, PlayMode::OPP_KICK_OFF}},
            {"kick_off_r", {PlayMode::OPP_KICK_OFF, PlayMode::OPP_KICK_OFF}}

        }};

        for(const auto& elemento : dict_play_modes) {
            if(elemento.first == key) {
                return elemento.second[Environment::is_left];
            }
        }

        return std::nullopt;
    }
    inline static PlayMode pm {};

    // ----- Atributos Únicos a Cada Jogador
    /** @brief ID do jogador atribuído pelo servidor. */
    uint8_t unum {};

    /** @brief Configuração de visão. [0]: Qualidade (High/Low), [1]: Largura (Narrow/Normal/Wide). */
    std::array<int, 2> view_mode {};

    /** @brief Gestão de energia. [0]: Stamina, [1]: Effort, [2]: Capacity. */
    std::array<float, 3> stamina_info {};
    inline static size_t stamina_max {8000}; // Assumiremos que será esse valor para todos.

    /** @brief Vetor de Posição do Jogador, será atualizado em Localizer {x_cart, y_cart, theta_x_hat} */
    std::array<double, 3> position {};

    /** @brief Vetor de Velocidade do Jogador {vel_x, vel_y} */
    std::array<double, 2> vector_vel {};

    /** @brief Vetor Polar velocidade relativo ao campo. {|vel|, vel_hat} */
    std::array<double, 2> speed = {};

    /** @brief Ângulo do pescoço relativo ao torso. Persiste após comando 'turn'. */
    double head_angle {};

    /** @brief Point-to. [0]: Movable, [1]: Expires, [2,3]: Target(X,Y). */
    std::array<double, 4> arm {};

    /** @brief Foco sensorial, habilidade do jogador. [0]: Tipo, [1,2]: Meta(ID/XY), [3,4]: Pos(XY).
    todo: Necessário novo estudo sobre esse carinha. Descobriu-se que ele na verdade não é o que imaginávamos

    - O Focus Point pode ser continuamente reposicionado para coincidir com a posição observada da bola.
    Embora o servidor não possua um comando específico para "focar na bola", basta utilizar sua distância
    e direção atuais como parâmetros do comando change_focus. Essa estratégia reduz o ruído aplicado às
    observações da bola, tornando sua posição e trajetória mais precisas durante conduções, interceptações
    e finalizações.

    O comando change_focus pode ser enviado a cada ciclo de simulação, permitindo que o agente altere
    continuamente a região de maior precisão visual conforme o contexto da partida. O foco pode migrar
    da bola para um companheiro, deste para um adversário e, posteriormente, para um landmark,
    transformando o Focus Point em um mecanismo de atenção visual dinâmica semelhante ao comportamento
    observado em jogadores humanos.
    */
    std::array<double, 5> focus {};

    /** @brief Status de faltas. [0]: Ativo, [1]: Cartão tomado. */
    std::array<double, 2> fouls {};

    /**
     * @brief Empacota até 4 caracteres em um único inteiro de 32 bits.
     * token1 -> byte mais significativo
     * token4 -> byte menos significativo
     * @return Valor de 32 bits contendo os caracteres empacotados.
     */
    inline static constexpr uint32_t pack(const char& token1, const char& token2 = 0, const char& token3 = 0, const char& token4 = 0) {
        return (static_cast<uint32_t>(token1) << 24) |
               (static_cast<uint32_t>(token2) << 16) |
               (static_cast<uint32_t>(token3) << 8)  |
               (static_cast<uint32_t>(token4));
    }
    /**
     * @brief Converte uma sequência de até 4 tokens em um index compacto.
     * @details
     * Realiza o empacotamento dos tokens utilizando Environment::pack()
     * e busca o valor correspondente na tabela estática de flags.
     * @return ID correspondente no intervalo [0, 59].
     * @return 255 caso a sequência não exista na tabela.
     */
    inline static uint8_t tokenstoid(const char& token1, const char& token2 = 0, const char& token3 = 0, const char& token4 = 0) {
        static constexpr std::array<uint32_t, 60> flagtable = {
            Environment::pack('b'),                    // 0
            Environment::pack('f', 'b', '0'),          // 1
            Environment::pack('f', 'b', 'l', '1'),     // 2
            Environment::pack('f', 'b', 'l', '2'),     // 3
            Environment::pack('f', 'b', 'l', '3'),     // 4
            Environment::pack('f', 'b', 'l', '4'),     // 5
            Environment::pack('f', 'b', 'l', '5'),     // 6
            Environment::pack('f', 'b', 'r', '1'),     // 7
            Environment::pack('f', 'b', 'r', '2'),     // 8
            Environment::pack('f', 'b', 'r', '3'),     // 9
            Environment::pack('f', 'b', 'r', '4'),     // 10
            Environment::pack('f', 'b', 'r', '5'),     // 11
            Environment::pack('f', 'c'),               // 12
            Environment::pack('f', 'c', 'b'),          // 13
            Environment::pack('f', 'c', 't'),          // 14
            Environment::pack('f', 'g', 'l', 'b'),     // 15
            Environment::pack('f', 'g', 'l', 't'),     // 16
            Environment::pack('f', 'g', 'r', 'b'),     // 17
            Environment::pack('f', 'g', 'r', 't'),     // 18
            Environment::pack('f', 'l', '0'),          // 19
            Environment::pack('f', 'l', 'b'),          // 20
            Environment::pack('f', 'l', 'b', '1'),     // 21
            Environment::pack('f', 'l', 'b', '2'),     // 22
            Environment::pack('f', 'l', 'b', '3'),     // 23
            Environment::pack('f', 'l', 't'),          // 24
            Environment::pack('f', 'l', 't', '1'),     // 25
            Environment::pack('f', 'l', 't', '2'),     // 26
            Environment::pack('f', 'l', 't', '3'),     // 27
            Environment::pack('f', 'p', 'l', 'b'),     // 28
            Environment::pack('f', 'p', 'l', 'c'),     // 29
            Environment::pack('f', 'p', 'l', 't'),     // 30
            Environment::pack('f', 'p', 'r', 'b'),     // 31
            Environment::pack('f', 'p', 'r', 'c'),     // 32
            Environment::pack('f', 'p', 'r', 't'),     // 33
            Environment::pack('f', 'r', '0'),          // 34
            Environment::pack('f', 'r', 'b'),          // 35
            Environment::pack('f', 'r', 'b', '1'),     // 36
            Environment::pack('f', 'r', 'b', '2'),     // 37
            Environment::pack('f', 'r', 'b', '3'),     // 38
            Environment::pack('f', 'r', 't'),          // 39
            Environment::pack('f', 'r', 't', '1'),     // 40
            Environment::pack('f', 'r', 't', '2'),     // 41
            Environment::pack('f', 'r', 't', '3'),     // 42
            Environment::pack('f', 't', '0'),          // 43
            Environment::pack('f', 't', 'l', '1'),     // 44
            Environment::pack('f', 't', 'l', '2'),     // 45
            Environment::pack('f', 't', 'l', '3'),     // 46
            Environment::pack('f', 't', 'l', '4'),     // 47
            Environment::pack('f', 't', 'l', '5'),     // 48
            Environment::pack('f', 't', 'r', '1'),     // 49
            Environment::pack('f', 't', 'r', '2'),     // 50
            Environment::pack('f', 't', 'r', '3'),     // 51
            Environment::pack('f', 't', 'r', '4'),     // 52
            Environment::pack('f', 't', 'r', '5'),     // 53
            Environment::pack('g', 'l'),               // 54
            Environment::pack('g', 'r'),               // 55
            Environment::pack('l', 'b'),               // 56
            Environment::pack('l', 'l'),               // 57
            Environment::pack('l', 'r'),               // 58
            Environment::pack('l', 't'),               // 59
        };

        // Caso seja para obtermos o index correspondente a um jogador
        if(token1 == 'p') {
            /* As seguintes parcelas representam:
            60          -> 0...59 representam a bola e as flags de campo, logo a primeira posição válida é 60.
            token2 * 11 -> Caso sejam aliados, token2 = 0. Logo, os primeiros slots são para aliados. Mesma lógica para adversários.
            token3 - 1  -> número do jogador representará sua posição no array
            */
            return 60 + (token2 * 11) + (token3 - 1);
        }
        uint32_t target = Environment::pack(token1, token2, token3, token4);
        // Busca Binária safamente
        auto it = std::lower_bound(
            flagtable.begin(),
            flagtable.end(),
            target
        );
        if (it != flagtable.end() && *it == target) {
            // Retorna a distância do início, que é o seu index no array
            return static_cast<uint8_t>(std::distance(flagtable.begin(), it));
        }
        return 255;
    }
    /** @brief Struct que representará pontos observados pelo jogador */
    struct Point {
        /** @brief Array que armazenará informações do que foi observado
            @details Dependendo do que é, armazenará mais informações

        Ponto Fixo / Bola -
            Perto: {DistRel, DirRel, DistChange, DirChange, 99, 99}
            Longe: {DistRel, DirRel,         99,        99, 99, 99}
        Jogador -
            Perto: {DistRel, DirRel, DistChange, DirChange, bodyDir, headDir}
            Longe: {DistRel, DirRel, DistChange, DirChange,      99,      99}
            Suficientemente longe: {DistRel, DirRel, 99, ...}

        Acredito que esses valores de `Change`, o qual significa variação, são interessantes
        no contexto de controle de erro PID e podem ser usados futuramente.
         */
        std::array<double, 6> attrs;
        /** @brief Array que armazenará a posição cartesiana relativo do ponto {Px_rel, Py_rel} ao jogador */
        std::array<double, 2> pos_cart_rel;
        /** @brief Array que armazenará a posição cartesiana absoluta do ponto {Px, Py} ao centro */
        std::array<double, 2> pos_cart_abs;

        /* Acredito que seja bom deixarmos em struct para posterior adição de funcionalidades */
    };
    /** @brief Array que armazenará todas os pontos possíveis e seus respectivos dados */
    std::array<Point, 60 + 11 * 2> points_on_the_field {
        /* Bola */
        /* Landmarks */
        /* Players */
    };
    /** @brief Array que  armazenará o index de todos os pontos vísiveis no momento */
    std::array<int, 60 + 11 * 2> visibles_index {};
    /** @brief Variável que nos possibilitará não apagar e repopular array sempre */
    int number_visibles {};

    /**
     * @brief Agrupa todas as funcionalidades de interpretação das mensagens do servidor.
     */
    class WorldParser {
    private:

        /** @brief Atributo que marca o ponto da mensagem que está sendo lido */
        const char* cursor = {nullptr};
        /** @brief Marcador do final da mensagem */
        const char* end    = {nullptr};

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
         * @brief Avança o cursor ignorando blocos aninhados "()" desconhecidos até fechar quantidade de '(' abertos inicialmente.
         * @details Não verifica se chegou ao final da string, o que corrobora Segmentation Fault
         * @param init_count Número inicial de pares a ignorar (padrão=1).
         */
        void skip_unknown(int init_count = 1) {

            int count_pair = init_count;
            while(count_pair != 0) {
                count_pair += (*this->cursor == '(') * (1) + (*this->cursor == ')') * (-1);
                this->cursor++;
            }
        }

        /**
         * @brief Interpreta mensagem `sense_body` do servidor.
         *
         * @param env Ambiente onde os dados serão armazenados.
         *
         * @details Processa:
         * - `view_mode`
         * - `stamina`, `speed`, `head_angle`
         * - `arm`
         * - `focus`
         * - `foul`
         * - `focus_point`
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
                    case 'v': { // `view_mode`

                        // Informação da qualidade da visão: `low` ou `high`
                        env.view_mode[0] = *(++this->cursor) == 'l';
                        this->cursor += 6 - env.view_mode[0];

                        // Informação de visão angular
                        switch(*this->cursor) {
                            case 'a': { // `narrow`
                                env.view_mode[1] = 0;
                                break;
                            }
                            case 'o': { // `normal`
                                env.view_mode[1] = 1;
                                break;
                            }
                            case 'i': { // `wide`
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

                            GeneralMath::transform_polar_to_cartesian_relative(
                                env.speed,
                                env.vector_vel
                            );
                        }
                        this->skip_until_char(')');
                        break;
                    }
                    case 'h': { // `head_angle`

                        std::string_view str_head_angle = this->get_next_str();
                        std::from_chars(
                            str_head_angle.data(),
                            str_head_angle.data() + str_head_angle.size(),
                            env.head_angle
                        );
                        this->cursor++;

                        /* Sabemos que agora vem os ActionCounters
                        Como são informações inúteis, vamos apenas
                        pulá-los
                        */
                        float total_actioncounters = 9.0;
                        while(total_actioncounters) {
                            if(*this->cursor == '(' || *this->cursor == ')') {
                                total_actioncounters -= 0.5;
                            }
                            this->cursor++;
                        }
                        break;
                    }
                    case 'a': { // `arm`

                        for(int i = 0; i < 3; ++i) {

                            std::string_view str_value = this->get_next_str();
                            if(str_value[0] == 't') { // `target`
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
                            else { // `movable` `expires`
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
                    case 'f': { // `focus` `fouls` `focus_point`
                        switch(lower_tag.size()) {
                            case 5: { // `focus`

                                // Pular o próximo nome, pois sabe-se que é target
                                this->get_next_str();
                                // Obter o objeto de foco
                                std::string_view str_value = this->get_next_str();
                                switch(str_value[2]) {
                                    case 'n': { // `none`

                                        env.focus[0] = 0;
                                        this->cursor++;
                                        break;
                                    }
                                    case 'l': { // `ball`

                                        env.focus[0] = 1;
                                        this->cursor++;
                                        break;
                                    }
                                    case 'a': { // `player`

                                        env.focus[0] = 2;
                                        env.focus[1] = (
                                            (*(this->cursor++) == 'r') && Environment::is_left
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
                                    case 'i': { // `point`

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
                            case 4: { // `foul`

                                // Captação de Faltas Recentes
                                this->get_next_str();
                                this->cursor++;
                                env.fouls[0] = *(this->cursor++) - 48;
                                this->cursor++;
                                // Cor de Cartão
                                this->get_next_str();
                                std::string_view str_value = this->get_next_str();
                                switch(str_value[0]) {
                                    case 'n': { // `none`
                                        env.fouls[1] = 0;
                                        break;
                                    }
                                    case 'y': { // `yellow`
                                        env.fouls[1] = 1;
                                        break;
                                    }
                                    case 'r': { // `red`
                                        env.fouls[1] = 2;
                                        break;
                                    }
                                }
                                this->cursor += 3;
                                break;
                            }
                            case 11: { // `focus_point`
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
                        if(!lower_tag.size()) {
                            return;
                        }
                        break;
                    }
                }
            }
        }

        /**
         * @brief Processa a mensagem `see` do servidor.
         * @details
         * Extrai os objetos visíveis, converte seus tokens em IDs internos
         * e atualiza os atributos dos pontos visualizados no ambiente.
         *
         * @param env Ambiente que receberá os dados processados.
         */
        void parse_see(Environment& env) {

            // Pulamos a informação do ciclo
            this->get_next_str();

            // Reiniciamos as variáveis necessárias
            env.number_visibles = {};
            std::array<char, 4> tokens {};
            int number_tokens {};
            while(*this->cursor != ')' && (this->cursor + 2) < this->end) {

                ///////////////////////////////////////////////////
                /* Obtenção de tokens */
                ///////////////////////////////////////////////////

                // Aplicamos um deslocamento por conta dos espaçamentos que existem na mensagem `see`
                this->cursor += 2;

                // Reiniciamos variáveis para o loop
                number_tokens = 0;
                tokens[0] = 0; tokens[1] = 0; tokens[2] = 0; tokens[3] = 0;
                while(*(this->cursor++) != ')') {

                    // Caso seja informações de jogador
                    if(*this->cursor == 'p' && !number_tokens) {
                        // Temos algo como: (p "RoboIME" 8)

                        // Pegamos o p
                        tokens[number_tokens++] = *(this->cursor++);

                        // Pegamos o nome do time
                        std::string_view team_name = this->get_next_str();

                        // Realizamos um tratamento rápido no nome
                        team_name.remove_prefix(1);
                        team_name.remove_suffix(1);
                        tokens[number_tokens++] = team_name != Booting::TEAMNAME;

                        // O número do jogador pode ser 10 ou 11
                        std::string_view str_value = this->get_next_str();
                        tokens[number_tokens++] = static_cast<char>(std::atoi(str_value.data()));

                        // Às vezes vem a informação se é `goalie`, o que é inútil para o que estamos fazendo no momento
                        // que é apenas obter uma ordenação no array

                        this->skip_until_char(')');
                        break;
                    }

                    // Caso não seja jogador, será ou flag de campo ou bola, que vem apenas em caracteres simples
                    tokens[number_tokens++] = *(this->cursor++);

                    /*
                    Isso é para casos específicos nos quais os tokens são "(f r t 10)"
                    Observe que o cursor parou em '0'. Daí, avançamos para o início das leituras
                    dos valores do observado
                    */
                    if(*this->cursor == '0') {
                        this->cursor += 2;
                        break;
                    }
                }

                //////////////////////////////////////////
                /* Obtenção do Elemento Observado */
                //////////////////////////////////////////

                // Verificação de Erros de Invalidez
                if(
                    /*
                    Em condições específicas o jogador reconhece elementos da seguinte forma:
                        (F)   // Flag desconhecida
                        (G)   // Gol desconhecido
                        (B)   // Bola (em algumas situações de baixa qualidade da visão)
                        (P)   // Jogador desconhecido
                        (L)   // Linha desconhecida (dependendo da versão/configuração)
                    Usamos essa primeira condição para impedir que haja tentativa de compreensão dessas observações
                    */
                    (number_tokens < 2 && tokens[0] != 'b') ||
                    /*
                    Caso algum jogador chegue com menos informações que o mínimo:
                        (p)
                        (p "TEAM")
                        (p "TEAM" 7)
                    Observe que no caso que há apenas o (p), faz-se necessário um avanço para permitir o funcionamento correto
                    de `skip_until_char`.
                    */
                    (number_tokens < 3 && tokens[0] == 'p')
                ) {
                    if(tokens[0] == 'p') {
                        this->cursor++;
                    }
                    this->skip_until_char(')');
                    continue;
                }

                // Obtemos o index correspondente
                uint8_t index = Environment::tokenstoid(
                    tokens[0],
                    tokens[1],
                    tokens[2],
                    tokens[3]
                );

                // Verificações a existência dela
                if(index == 255) {
                    env.logger.warn(
                        "Conjunto Inválido em parse_see: ({}, {}, {}, {})",
                        tokens[0],
                        tokens[1],
                        tokens[2],
                        tokens[3]
                    );
                    this->skip_until_char(')');
                    continue;
                }

                // Então o ponto existe e foi visualizado
                Environment::Point& point = env.points_on_the_field[index];
                env.visibles_index[env.number_visibles++] = index;

                //////////////////////////////////////////////////////////////////
                /* Começamos a obtenção das informações do ponto observado */
                //////////////////////////////////////////////////////////////////

                int i = 0;
                while(*this->cursor != ')') {
                    std::string_view str_value = this->get_next_str();
                    std::from_chars(
                        str_value.data(),
                        str_value.data() + str_value.size(),
                        point.attrs[i++]
                    );
                }
                while(i != static_cast<int>(point.attrs.size())) {
                    // Quando não tiver os valores para atualizar, apenas diremos que se trata de um valor específico
                    point.attrs[i++] = 99;
                }

                GeneralMath::transform_polar_to_cartesian_relative(
                    // Fazemos span para forçar a visualização de apenas os dois primeiros elementos, os quais são comuns
                    // a qualquer observado, sendo distância relativa e ângulo relativo
                    std::span<double, 2>(point.attrs.data(), 2),
                    point.pos_cart_rel
                );

                this->cursor++;
            }
        }

        /**
         * @brief Processa mensagens do tipo "hear" recebidas do servidor.
         *
         * Analisa o remetente da mensagem e executa ação apropriada:
         * - referee: atualiza modo de jogo
         * - self: eco de mensagem própria
         * - online_coach_left: mensagem do coach
         * - número/'-': mensagem de outro jogador com ângulo
         * - outros: ignora desconhecidos
         *
         * @param env Ambiente a ser atualizado (modo de jogo, logger).
         */
        void parse_hear(Environment& env) {
            // Então sabemos que virá informações de ciclos
            this->get_next_str();

            // Teremos o sender
            std::string_view sender = this->get_next_str();

            // Com o sender, virão as possibilidades
            switch(sender[0]) {

                case 'r': { // referee

                    // referee mandou o modo de jogo
                    std::string_view possible_mode = this->get_next_str();
                    std::optional<Environment::PlayMode> result_from_search = Environment::get_play_mode(possible_mode);
                    if(result_from_search.has_value()) {
                        // std::move para não haver cópia
                        Environment::pm = std::move(result_from_search.value());
                    }
                    else {
                        env.logger.warn("Mode Unknown: {}", possible_mode);
                    }

                    return; // Podemos apenas encerrar de uma vez, pois essas mensagens vem isoladas
                }
                case 's': { // self
                    // Enviamos uma mensagem `say` e escutamos ela

                    return;
                }
                case 'o': { // online_coach_left
                    // coach envia mensagens aos jogadores

                    return;
                }
                default: {
                    if(sender[0] == '-' || (sender[0] >= 49 && sender[0] <= 57)) {
                        /**
                        Então `sender` representa o ângulo no qual a mensagem foi ouvida.
                        E a próxima string é a mensagem!
                        */

//                         std::string_view message_heard = this->get_next_str();

                        return;
                    }
                    this->skip_unknown();
                    return;
                }
            }
        }

        /**
         * @brief Reinicia os ponteiros da região atual.
         * @details Atribui `nullptr` a ambos os ponteiros (início e fim)
         *          para evitar acesso indevido a memória.
         */
        void clean() {
            this->cursor = nullptr;
            this->end    = nullptr;
        }

        /**
         * @brief Extrai uma string da região de memória.
         * @return std::string 50 caracteres, começando 20 posições antes do cursor.
         * @warning Assume que `cursor - 20` é um endereço válido, mas pode não ser
         */
        std::string get_region() {
            return std::string(std::string_view(this->cursor - 6, 16));
        }

    public:

        /**
         * @brief A partir de mensagens do servidor, atualizará os dados do ambiente.
         */
        void update_from_server(
            const std::string_view& message_from_server,
            Environment& env
        ) {
            // Definimos atributos cruciais
            this->cursor = message_from_server.data();
            this->end    = message_from_server.data() + message_from_server.size();

            if(this->cursor == nullptr) {
                return;
            }

            std::string_view uppest_tag = this->get_next_str();
            switch (uppest_tag[0]) {

                case 'i': { // init

                    if(env.unum != 1) {
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
                        // std::move para não haver cópia
                        Environment::pm = std::move(result_from_search.value());
                    }
                    else {
                        env.logger.warn("Mode Unknown: {}", possible_mode);
                    }

                    return this->clean();
                }

                case 's': { // server_param see sense_body

                    switch (uppest_tag.size()) {

                        case 3: { // see

                            this->parse_see(env);
                            return this->clean();
                        }

                        case 10: { // sense_body

                            this->parse_sensebody(env);
                            return this->clean();
                        }

                        case 12: { // server_param
                            return this->clean(); // Vamos apenas pular essa mensagem
                        }
                    }

                    break;
                }

                case 'h': { // hear
                    // Por enquanto, vamos apenas ignorar esse tipo de mensagem.
                    // Mas é fato que é importante e merece atenção futura.
                    this->parse_hear(env);
                    return this->clean();
                }

                case 'p': { // player_param player_type
                    return this->clean(); // Vamos apenas pular essa mensagem
                }

                case 'o': { // ok
                    return this->clean(); // Vamos apenas pular essa mensagem
                }

                case 'w': { // warning
                    return this->clean(); // Vamos apenas pular essa mensagem
                }

                case 'c': { // change_player_type
                    return this->clean(); // Vamos apenas pular essa mensagem
                }

                default:
                    env.logger.warn("Uppest_Tag Unknown: {} | Redondezas: {}", uppest_tag, this->get_region());
                    // Como trata-se de uma tag superior desconhecida, podemos apenas jogar o restante fora
                    return this->clean(); // Vamos apenas pular essa mensagem
            }
        }
    };
    WorldParser wp;
};