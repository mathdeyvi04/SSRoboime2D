#pragma once

#include <string_view>
#include <cstdint>

#include "../communication/ServerComm.hpp"


class BasicAgent {
private:
    ServerComm __sc;


    uint8_t unum;
public:

    static uint8_t number_players = 0;

    BasicAgent() {
        // Inicializamos todas os pontos principais
        this->unum = ++number_players;

        // Teletransportamos o jogador para a posição correta
        this->beam(

        );
    };

    void beam(int posx, int posy, int angle){
         // It moves the player to the exact position of X (between −54 and 54) and Y (between −32 and 32) in one simulation cycle.


    }

    void run() {

        // Recebemos algo do servidor
        std::string_view this->__sc.receive();

        // Interpretamos a mensagem

        // Tomamos alguma decisão

    }


};