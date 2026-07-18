#pragma once

#include <array>

#include "./Environment.hpp"

class Localizer {
public:

    /** @brief Comprimento Total do Campo */
    inline static constexpr double WIDTH_FIELD {105.0};
    /** @brief Largura Total do Campo */
    inline static constexpr double HEIGHT_FIELD {68.0};
    /** @brief Vetor de Informações de Pontos Fixos no Campo que poderão ser usados para localização
        @details Contém triplas sobre {index_points_field, posx_abs, posy_abs}
     */
    inline static constexpr std::array<double, 27> info_landmarks {
        12, 0,                    0,                 // f c
        24, -WIDTH_FIELD / 2,     - HEIGHT_FIELD / 2,// f l t
        14, 0,                    - HEIGHT_FIELD / 2,// f c t
        39, WIDTH_FIELD / 2,      - HEIGHT_FIELD / 2,// f r t
        55, WIDTH_FIELD / 2,      0,                 // g r
        35, WIDTH_FIELD / 2,      HEIGHT_FIELD / 2,  // f r b
        13, 0,                    HEIGHT_FIELD / 2,  // f c b
        20, -WIDTH_FIELD / 2,     HEIGHT_FIELD / 2,  // f l b
        54, -WIDTH_FIELD / 2,     0,                 // g l
    };

    /** @brief Vetor que armazenará quais os indexs dos landmarks visíveis no array de informações de landmarks */
    std::array<int, 9> index_info_landmarks_visibles {};
    /** @brief Contador para quantos landmarks estão visíveis */
    int count_for_landmarks_visibles {};

    /**
     * @brief Verifica se um ponto visível é um landmark conhecido.
     * @param index_point_visible Índice do ponto observado no campo.
     * @return int 0 se o ponto é um landmark (registrado), 1 caso contrário.
     */
    int verify_landmarks(int index_point_visible) {

        // A fim de melhorar a performance...
        if(index_point_visible > 55 || index_point_visible < 12) {
            return 1;
        }

        for(int i = 0; i < static_cast<int>(info_landmarks.size()); i = i + 3) {
            if(info_landmarks[i] == index_point_visible) {
                // Achamos um landmark visível
                this->index_info_landmarks_visibles[
                    this->count_for_landmarks_visibles++
                ] = i;
                return 0;
            }
        }
        return 1;
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

    /**
     * @brief Atualiza posição do jogador usando landmarks visíveis com média ponderada.
     * @details Processa pares de landmarks para estimar pose e posição, acumulando resultados
     * com pesos baseados em distância e condicionamento geométrico.
     *
     * @param position_player [out] Posição estimada {x, y, pose} em graus.
     * @param points_on_the_field [in/out] Vetor de pontos do campo (landmarks + outros).
     * @return int 0 em sucesso, 1 se landmarks insuficientes (< 2).
     */
    int update_location(
        std::array<double, 3>& position_player,
        std::array<Environment::Point, 60 + 11 * 2>& points_on_the_field
    ) {

        // Caso não tenha a quantidade mínima de landmarks, não será possível
        if(this->count_for_landmarks_visibles < 2) {
            return 1;
        }

        double sum_weighted_sin {};
        double sum_weighted_cos {};
        std::array<double, 2> sum_weighted_positions {};
        double sum_weights {};
        for(int i = 0; i < this->count_for_landmarks_visibles - 1; ++i) {
            for(int j = i + 1; j < this->count_for_landmarks_visibles; ++j) {
                // Acessaremos pares (i, j) dentro de index_info_landmarks_visibles
                // Ex: (0,1), (0,2), (1,2), (0,3), (1,3), (2,3)...

                // Acessamos os landmarks visualizados
                Environment::Point& landmark1 = points_on_the_field[Localizer::info_landmarks[index_info_landmarks_visibles[i]]];
                landmark1.pos_cart_abs[0] = Localizer::info_landmarks[index_info_landmarks_visibles[i] + 1];
                landmark1.pos_cart_abs[1] = Localizer::info_landmarks[index_info_landmarks_visibles[i] + 2];
                Environment::Point& landmark2 = points_on_the_field[Localizer::info_landmarks[index_info_landmarks_visibles[j]]];
                landmark2.pos_cart_abs[0] = Localizer::info_landmarks[index_info_landmarks_visibles[j] + 1];
                landmark2.pos_cart_abs[1] = Localizer::info_landmarks[index_info_landmarks_visibles[j] + 2];

                // Para os pontos acima, obtemos os valores de posição e de pose
                std::array<double, 3> position_and_pose = this->calculate_position_and_pose(
                    landmark1.pos_cart_rel,
                    landmark1.pos_cart_abs,
                    landmark2.pos_cart_rel,
                    landmark2.pos_cart_abs
                );

                /**
                Acumularemos o resultado com os demais produzidos pelos outros pares
                a fim de conseguirmos mais confiança!
                Para tanto, usaremos médis ponderadas onde o peso leva em consideração
                o inverso da distância e o condicionamento geométrico da dupla
                 */
                double weight = std::abs(std::sin(landmark2.attrs[1] - landmark1.attrs[1])) /
                                (landmark2.attrs[0] + landmark1.attrs[0]);

                // Relacionado à pose
                sum_weighted_sin += weight * std::sin(position_and_pose[2]);
                sum_weighted_cos += weight * std::cos(position_and_pose[2]);

                // Relacionados à posição
                sum_weighted_positions[0] += weight * position_and_pose[0];
                sum_weighted_positions[1] += weight * position_and_pose[1];
                sum_weights += weight;
            }
        }

        // Após os loops terem sido executados, é possível:
        position_player[0] = sum_weighted_positions[0] / sum_weights;
        position_player[1] = sum_weighted_positions[1] / sum_weights;
        position_player[2] = std::atan2(sum_weighted_sin, sum_weighted_cos);
        return 0;
    }
};