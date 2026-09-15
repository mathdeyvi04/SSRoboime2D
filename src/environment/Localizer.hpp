#pragma once

#include <array>
#include <chrono>

#include "./Environment.hpp"

// todo necessary: Precisamos inserir a possibilidade de linhas no cálculo
class Localizer {
public:

    /** @brief Comprimento Total do Campo */
    inline static constexpr double WIDTH_FIELD {105.0};
    /** @brief Largura Total do Campo */
    inline static constexpr double HEIGHT_FIELD {68.0};
    /** @brief Posição X da Linha Lateral da Área de Pênalti */
    inline static constexpr double X_POSITION_PENALTI_AREA {36.2};
    /** @brief Altura do Ponto Superior Esquerdo da Área de Pênalti */
    inline static constexpr double HEIGHT_HIGHER_POINT_IN_PENALTI_AREA {20.0};
    /** @brief Distância da Linha Externa ao Campo */
    inline static constexpr double DIST_OFF_FIELD {5.0};

    /** @brief Vetor de Informações de Pontos Fixos no Campo que poderão ser usados para localização
        @details Contém triplas sobre {index_points_field, posx_abs, posy_abs}
     */
    inline static constexpr auto INFO_LANDMARKS = std::to_array<double>({
        // f c
        12, 0,                    0,
        // f l t
        24, -WIDTH_FIELD / 2,     -HEIGHT_FIELD / 2,
        // f c t
        14, 0,                    -HEIGHT_FIELD / 2,
        // f r t
        39, WIDTH_FIELD / 2,      -HEIGHT_FIELD / 2,
        // g r
        55, WIDTH_FIELD / 2,      0,
        // f r b
        35, WIDTH_FIELD / 2,      HEIGHT_FIELD / 2,
        // f c b
        13, 0,                    HEIGHT_FIELD / 2,
        // f l b
        20, -WIDTH_FIELD / 2,     HEIGHT_FIELD / 2,
        // g l
        54, -WIDTH_FIELD / 2,     0,
        // f p r t
        33, X_POSITION_PENALTI_AREA,  -HEIGHT_HIGHER_POINT_IN_PENALTI_AREA,
        // f p r c
        32, X_POSITION_PENALTI_AREA,  0,
        // f p r b
        31, X_POSITION_PENALTI_AREA,  HEIGHT_HIGHER_POINT_IN_PENALTI_AREA,
        // f p l t
        30, -X_POSITION_PENALTI_AREA, -HEIGHT_HIGHER_POINT_IN_PENALTI_AREA,
        // f p l c
        29, -X_POSITION_PENALTI_AREA, 0,
        // f p l b
        28, -X_POSITION_PENALTI_AREA, HEIGHT_HIGHER_POINT_IN_PENALTI_AREA,

        // Pontos Fora do Campo
        // f l t 10
        25, -WIDTH_FIELD / 2 - DIST_OFF_FIELD, -10,
        // f l t 20
        26, -WIDTH_FIELD / 2 - DIST_OFF_FIELD, -20,
        // f l t 30
        27, -WIDTH_FIELD / 2 - DIST_OFF_FIELD, -30,

        // f l b 10
        21, -WIDTH_FIELD / 2 - DIST_OFF_FIELD, 10,
        // f l b 20
        22, -WIDTH_FIELD / 2 - DIST_OFF_FIELD, 20,
        // f l b 30
        23, -WIDTH_FIELD / 2 - DIST_OFF_FIELD, 30,

        // f t l 10
        44, -10, -HEIGHT_FIELD / 2 - DIST_OFF_FIELD,
        // f t l 20
        45, -20, -HEIGHT_FIELD / 2 - DIST_OFF_FIELD,
        // f t l 30
        46, -30, -HEIGHT_FIELD / 2 - DIST_OFF_FIELD,
        // f t l 40
        47, -40, -HEIGHT_FIELD / 2 - DIST_OFF_FIELD,

        // f t r 10
        49, 10, -HEIGHT_FIELD / 2 - DIST_OFF_FIELD,
        // f t r 20
        50, 20, -HEIGHT_FIELD / 2 - DIST_OFF_FIELD,
        // f t r 30
        51, 30, -HEIGHT_FIELD / 2 - DIST_OFF_FIELD,
        // f t r 40
        52, 40, -HEIGHT_FIELD / 2 - DIST_OFF_FIELD,

        // f r t 10
        40, WIDTH_FIELD / 2 + DIST_OFF_FIELD, -10,
        // f r t 20
        41, WIDTH_FIELD / 2 + DIST_OFF_FIELD, -20,
        // f r t 30
        42, WIDTH_FIELD / 2 + DIST_OFF_FIELD, -30,

        // f r b 10
        36, WIDTH_FIELD / 2 + DIST_OFF_FIELD, 10,
        // f r b 20
        37, WIDTH_FIELD / 2 + DIST_OFF_FIELD, 20,
        // f r b 30
        38, WIDTH_FIELD / 2 + DIST_OFF_FIELD, 30,

        // f b r 10
        7, 10, HEIGHT_FIELD / 2 + DIST_OFF_FIELD,
        // f b r 20
        8, 20, HEIGHT_FIELD / 2 + DIST_OFF_FIELD,
        // f b r 30
        9, 30, HEIGHT_FIELD / 2 + DIST_OFF_FIELD,
        // f b r 40
        10, 40, HEIGHT_FIELD / 2 + DIST_OFF_FIELD,

        // f b l 10
        2, -10, HEIGHT_FIELD / 2 + DIST_OFF_FIELD,
        // f b l 20
        3, -20, HEIGHT_FIELD / 2 + DIST_OFF_FIELD,
        // f b l 30
        4, -30, HEIGHT_FIELD / 2 + DIST_OFF_FIELD,
        // f b l 40
        5, -40, HEIGHT_FIELD / 2 + DIST_OFF_FIELD
    });
    /**@brief Array para não precisarmos percorrer todos landmarks possíveis */
    inline static constexpr auto LOOKUP_TABLE_TO_INFO_LANDMARKS = []() {
        std::array<int, 56> lut {};
        lut.fill(-1);
        for(int i = 0; i < static_cast<int>(INFO_LANDMARKS.size()); i += 3) {
            int idx = static_cast<int>(INFO_LANDMARKS[i]);
            lut[idx] = i;
        }
        return lut;
    } (); // Com isso executamos essa função anônima
    /** @brief Vetor que armazenará quais os indexs dos landmarks visíveis no array de informações de landmarks */
    std::array<int, static_cast<int>(INFO_LANDMARKS.size() / 3)> m_index_info_landmarks_visibles {};
    /** @brief Contador para quantos landmarks estão visíveis */
    int m_count_for_landmarks_visibles {};
    /** @brief Número Máximo de Landmarks que serão utilizados pelo algoritmo */
    inline static constexpr int MAX_NUMBER_LANDMARKS_FOR_LOCALIZATION {8};
    /** @brief Máximo número de iterações do algoritmo de localização */
    inline static constexpr int MAX_NUMBER_ITERATIONS_FOR_LOCALIZATION {MAX_NUMBER_LANDMARKS_FOR_LOCALIZATION * (MAX_NUMBER_LANDMARKS_FOR_LOCALIZATION - 1) / 2};
    /** @brief Medidor de Confiança da Estimativa */
    int m_confidence {0};

    /**
     * @brief Inicializa as posições absolutas dos landmarks.
     * @details Pensou-se em fazer isso em tempo de compilação, mas o points_on_the_field ainda não estaria gerado!
     *
     * @param points_on_the_field Vetor com os pontos do campo (landmarks e linhas)
     *        cujas posições absolutas serão preenchidas.
     * @note O laço percorre @ref INFO_LANDMARKS em passos de 3, garantindo que
     *       cada iteração trate exatamente uma tripla `{ índice, x, y }`.
     */
    Localizer(
        std::array<Environment::Point, 60 + 11 * 2>& points_on_the_field
    ) {

        // Devemos popular as informações absolutas de todas os landmarks e linhas
        for(int i = 0; i < static_cast<int>(INFO_LANDMARKS.size()); i = i + 3) {
            Environment::Point& landmark = points_on_the_field[Localizer::INFO_LANDMARKS[i]];
            landmark.pos_cart_abs[0] = Localizer::INFO_LANDMARKS[i + 1];
            landmark.pos_cart_abs[1] = Localizer::INFO_LANDMARKS[i + 2];
        }
    }
    ~Localizer() = default;

    /**
     * @brief Verifica se um ponto visível é um landmark conhecido.
     * @param index_point_visible Índice do ponto observado no campo.
     * @return int 0 se o ponto é um landmark (registrado), 1 caso contrário.
     */
    int verify_landmarks(int index_point_visible) {
        int offset = Localizer::LOOKUP_TABLE_TO_INFO_LANDMARKS[index_point_visible];
        if(offset < 0) {
            return 1;
        }
        m_index_info_landmarks_visibles[
            m_count_for_landmarks_visibles++
        ] = offset;
        return 0;
    }

    /**
     * @brief Calcula posição absoluta e orientação a partir de dois vetores (relativo e absoluto).
     *
     * @param vector1_rel Coordenadas relativas do ponto 1 (x, y).
     * @param vector1_abs Coordenadas absolutas do ponto 1 (x, y).
     * @param vector2_rel Coordenadas relativas do ponto 2 (x, y).
     * @param vector2_abs Coordenadas absolutas do ponto 2 (x, y).
     * @return std::array<double, 3> {posx, posy, pose}
     */
    inline static std::array<double, 3> calculate_position_and_pose(
        const std::array<double, 2>& vector1_rel,
        const std::array<double, 2>& vector1_abs,
        const std::array<double, 2>& vector2_rel,
        const std::array<double, 2>& vector2_abs
    ) {

        /**
        Obtemos o ângulo do pescoço em relação ao eixo x
        Trata-se da diferença entre o ângulo das componentes do vetor diferença absoluto
        e do vetor diferença relativo ao jogador.

        À princípio, poderíamos ter erro de divisão por zero. Entretanto, temos
        a garantia de que não serão pontos coincidentes!
         */
        double pose = std::atan2(
            vector2_abs[1] - vector1_abs[1],
            vector2_abs[0] - vector1_abs[0]
        )           - std::atan2(
            vector2_rel[1] - vector1_rel[1],
            vector2_rel[0] - vector1_rel[0]
        );

        /**
        Obtemos o vetor posição do jogador a partir do primeiro vetor como referência
        Trata-se de P_j = P_abs1 - R(theta)P_obs1
         */
        double posx = vector1_abs[0] -
                     (vector1_rel[0] * std::cos(pose) - vector1_rel[1] * std::sin(pose));

        double posy = vector1_abs[1] -
                     (vector1_rel[0] * std::sin(pose) + vector1_rel[1] * std::cos(pose));

        return {posx, posy, pose};
    }

    int update_location(
        std::array<double, 3>& position_player,
        std::array<Environment::Point, 60  + 11 * 2>& points_on_the_field
    ) {
        m_confidence = m_count_for_landmarks_visibles;

        if(m_confidence < 2) {
            return -1;
        }

        auto start = std::chrono::high_resolution_clock::now();

        // Para trabalharmos com as médias ponderadas
        double sum_weighted_x {}, sum_weight_x {};
        double sum_weighted_y {}, sum_weight_y {};
        double sum_weighted_sin {}, sum_weighted_cos {};
        int iteration {};
        // Pares de Landmarks
        for(int i = 0; i < m_count_for_landmarks_visibles - 1; ++i) {
            for(int j = i + 1; j < m_count_for_landmarks_visibles; ++j) {
                // Acessaremos pares (i, j) dentro de index_info_landmarks_visibles
                // Ex: (0,1), (0,2), (1,2), (0,3), (1,3), (2,3)...

                if(iteration >= MAX_NUMBER_ITERATIONS_FOR_LOCALIZATION) {
                    break;
                }

                // Acessamos os landmarks visualizados
                Environment::Point& landmark1 = points_on_the_field[Localizer::INFO_LANDMARKS[m_index_info_landmarks_visibles[i]]];
                Environment::Point& landmark2 = points_on_the_field[Localizer::INFO_LANDMARKS[m_index_info_landmarks_visibles[j]]];

                // Para os pontos acima, obtemos os valores de posição e de pose
                std::array<double, 3> position_and_pose = calculate_position_and_pose(
                    landmark1.pos_cart_rel,
                    landmark1.pos_cart_abs,
                    landmark2.pos_cart_rel,
                    landmark2.pos_cart_abs
                );

                /**
                 * Acumularemos o resultado com os demais produzidos pelos outros pares
                 * a fim de conseguirmos mais confiança!
                 * Para tanto, usaremos médis ponderadas onde o peso leva em consideração
                 * o inverso da distância e o condicionamento geométrico da dupla
                 */
                double weight = std::abs(std::sin(landmark2.attrs[1] - landmark1.attrs[1])) /
                                (landmark2.attrs[0] + landmark1.attrs[0]);

                // Relacionado à pose
                sum_weighted_sin += weight * std::sin(position_and_pose[2]);
                sum_weighted_cos += weight * std::cos(position_and_pose[2]);

                // Relacionados à posição
                sum_weighted_x += weight * position_and_pose[0];
                sum_weight_x   += weight;
                sum_weighted_y += weight * position_and_pose[1];
                sum_weight_y   += weight;
                ++iteration;
            }

            if(iteration >= MAX_NUMBER_ITERATIONS_FOR_LOCALIZATION) {
                break;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        position_player[0] = sum_weighted_x / sum_weight_x;
        position_player[1] = sum_weighted_y / sum_weight_y;
        double possible_pose = std::atan2(sum_weighted_sin, sum_weighted_cos);
        position_player[2] = (std::abs(possible_pose) < 1e-7) ? 0 : possible_pose;
        return duration;
    }

    /**
     * @brief Verifica se o jogador está fora do campo e se afastando dele.
     *
     * @param position_player Vetor {posx, posy, pose} com posição e orientação do jogador.
     *                      Assume-se que pose é o body_angle em coordenadas de mundo.
     * @param relative_angle Ângulo relativo ao body_angle (ex: pescoço) que define a direção de interesse.
     * @return true  se o jogador está fora do campo E apontando para fora.
     *         false se está dentro do campo ou, estando fora, aponta para o campo.
     */
    bool check_if_out_of_field(
        const std::array<double, 3>& position_player,
        double body_angle = 0.0
    ) {
        const double posx = position_player[0];
        const double posy = position_player[1];

        const double half_width  = (WIDTH_FIELD  - 4) / 2.0;
        const double half_height = (HEIGHT_FIELD - 6) / 2.0;

        const bool out_right  = posx >  half_width;
        const bool out_left   = posx < -half_width;
        const bool out_bottom = posy >  half_height;
        const bool out_top    = posy < -half_height;

        // Dentro do campo
        if(!out_right && !out_left && !out_bottom && !out_top) {
            return false;
        }

        // Ângulo-alvo: direção que o jogador deveria seguir para voltar
        double target_angle;
        if(out_bottom && out_right)      {target_angle = -135.0;}
        else if(out_bottom && out_left)  {target_angle =  -45.0;}
        else if(out_top    && out_right) {target_angle =  135.0;}
        else if(out_top    && out_left)  {target_angle =   45.0;}
        else if(out_bottom)              {target_angle =  -90.0;}
        else if(out_top)                 {target_angle =   90.0;}
        else if(out_right)               {target_angle =  180.0;}
        else                             {target_angle =    0.0;}  // out_left

        // Apontando para fora: body_angle está a mais de 90° do alvo
        const double diff = GeneralMath::normalize_angle(body_angle - target_angle);
        return std::abs(diff) > 90.0;
    }
};