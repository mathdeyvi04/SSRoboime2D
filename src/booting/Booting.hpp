#pragma once

namespace Booting {
    // Configurações Simples
    inline constexpr const char* TEAMNAME = "RoboIME";
    inline constexpr const char* IPSERVER = "127.0.0.1";
    inline constexpr int   PORTSERVER = 6000;

    // Configurações de ServerComm
    inline constexpr int SIZEBUFFER = 512;
    inline constexpr int TIMEOUTSOCKETSERVER = 50; // Em milisegundos
}
