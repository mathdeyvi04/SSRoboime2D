#pragma once

#include <cmath>
#include <span>

namespace GeneralMath {

    /**
     * @brief Array inteligente com inserção sequencial automática.
     *
     * Mantém um array de tamanho fixo e permite inserir valores
     * com incremento automático do índice.
     *
     * @tparam N Número máximo de elementos.
     */
    template<size_t N>
    struct smart_array {
    public:
        std::array<float, N> data {};
        smart_array() = default;
        ~smart_array() = default;
        /**
         * @brief Insere um valor na posição atual e avança o índice.
         *
         * @param i Índice atual (incrementado após a inserção).
         * @param value Valor a ser armazenado (convertido para float).
         */
        void set(int& i, double value) {
            data[i++] = static_cast<float>(value);
        }

        const float operator[](int i) {
            return data[i];
        }
    };

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

    /**
     * @brief Converte coordenadas polares para cartesianas (sistema relativo).
     *
     * @param[out] polar_vector_relative Entrada: {magnitude, ângulo_graus}.
     * @param[out] cartesian_vector_relative Saída: {x, y}.
     */
    inline void transform_polar_to_cartesian_relative(
        // Usamos esse span, de `include <span>`, por causa do point.attrs, que tem 4 elementos!
        std::span<double, 2> polar_vector_relative,
        std::array<double, 2>& cartesian_vector_relative
    ) {

        cartesian_vector_relative[0] = polar_vector_relative[0] * GeneralMath::cosd(polar_vector_relative[1]);
        cartesian_vector_relative[1] = polar_vector_relative[0] * GeneralMath::sind(polar_vector_relative[1]);
    }

    /**
     * @brief Normaliza um ângulo para o intervalo [-180, 180).
     *
     * @param angle Ângulo em graus (qualquer valor).
     * @return double Ângulo normalizado no intervalo [-180, 180).
     */
    inline double normalize_angle(double angle) {
        angle = std::fmod(angle + 180.0, 360.0);
        if(angle < 0.0) {
            angle += 360.0;
        }
        return angle - 180.0;
    }

    /**
     * @brief Calcula o ângulo direcional do vetor (x,y) em relação ao eixo x.
     * Os valores negativos são para correção dos eixos
     * @param x Coordenada x do vetor.
     * @param y Coordenada y do vetor.
     * @return double Ângulo em graus no intervalo [-180, 180].
     */
    inline double angle_of_vector(double x, double y) {
        double angle_rad = std::atan2(-y, -x);
        double angle_deg = angle_rad * 180.0 / PI;
        if (angle_deg > 180.0) {
            angle_deg -= 360.0;
        }
        if (angle_deg < -180.0) {
            angle_deg += 360.0;
        }
        return angle_deg;
    }
}
