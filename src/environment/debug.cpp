#include <fstream>
#include <string>
#include <string_view>
#include <algorithm>

#include "./Environment.hpp"

int main(int argc, char** argv) {

    // Apenas carregamos o arquivo com mensagens de exemplo
    std::ifstream file("./example_messages_from_server.txt");
    std::string linha;

    // Vamos criar um ambiente de teste
    Environment env;

    // Realizamos os testes
    while(std::getline(file, linha)) {

        linha.erase(
            std::remove_if(
                linha.begin(), linha.end(),
                [](char c) { return c == '\r' || c == '\n'; }
            ),
            linha.end()
        );

        std::string_view message_from_server(linha);  // view válida apenas dentro do loop

        env.wp.update_from_server(
            message_from_server,
            env
        );
    }

    return 0;
}