#pragma once

#include <cmath>
#include <span>

namespace GeneralMath {

    /** @brief Definição de alta precisão para cálculos trigonométricos */
    constexpr double PI = 3.14159265358979323846;

    /** @brief Constante para Evitarmos acúmulo de erros */
    constexpr double EPSILON = 1e-9;

    /**
     * @brief Calcula o seno de um ângulo em graus.
     * @param angle_degrees Ângulo em graus (ex: 45.0, 90.0, 180.0).
     * @return double Seno do ângulo (intervalo [-1.0, 1.0]).
     * @note Converte automaticamente graus para radianos.
     * @example sind(30.0)  // Retorna 0.5
     *          sind(180.0) // Retorna ~0.0
     */
    inline double sind(double angle_degrees) {
        double result = std::sin(
            angle_degrees * PI / 180.0  // Conversão para radianos
        );

        return (std::abs(result) < EPSILON) ? 0 : result;
    }

    /**
     * @brief Calcula o cosseno de um ângulo em graus.
     * @param angle_degrees Ângulo em graus (ex: 45.0, 90.0, 180.0).
     * @return double Cosseno do ângulo (intervalo [-1.0, 1.0]).
     * @note Converte automaticamente graus para radianos.
     * @example cosd(60.0)  // Retorna 0.5
     *          cosd(0.0)   // Retorna 1.0
     */
    inline double cosd(double angle_degrees) {
        double result = std::cos(
            angle_degrees * PI / 180.0  // Conversão para radianos
        );

        return (std::abs(result) < EPSILON) ? 0 : result;
    }

    inline int transform_polar_to_cartesian_relative(
        // Usamos esse span, de `include <span>`, por causa do point.attrs, que tem 4 elementos!
        std::span<double, 2> polar_vector_relative,
        std::array<double, 2>& cartesian_vector_relative
    ) {

        cartesian_vector_relative[0] = polar_vector_relative[0] * GeneralMath::cosd(polar_vector_relative[1]);
        cartesian_vector_relative[1] = polar_vector_relative[0] * GeneralMath::sind(polar_vector_relative[1]);
        return 0;
    }
}
