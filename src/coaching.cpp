#include "./booting/cxxopts.hpp"
#include "./agent/TrainerAgent.hpp"

/**
 * @brief Guia Completo de Comandos do Trainer — rcssserver2d
 *
 * Este documento descreve detalhadamente todos os comandos aceitos pelo cliente Trainer
 * no simulador RCSSServer2D (RoboCup 2D Soccer Simulation Server).
 *
 * O Trainer é um cliente especial com privilégios elevados, utilizado principalmente para:
 * - Controlar o estado da partida durante treinamentos.
 * - Teletransportar jogadores e a bola no campo.
 * - Modificar a stamina e os tipos heterogêneos dos agentes.
 * - Injetar cenários específicos de teste e simulação.
 *
 * =============================================================================
 * 1. INICIALIZAÇÃO E CONTROLE DE CONEXÃO
 * =============================================================================
 *
 * (init (version <VER>))
 *   Inicia a conexão do Trainer com o servidor na porta do trainer (padrão 6001).
 *
 *   Parâmetros:
 *     <VER>: Versão do protocolo utilizada (ex: 15, 16, 17, 18).
 *
 *   Exemplo:
 *     (init (version 18))
 *
 * (bye) / (done)
 *   Encerra a sessão do Trainer com o servidor.
 *
 *   Exemplo:
 *     (bye)
 *
 * =============================================================================
 * 2. CONTROLE DO MODO DE JOGO (PLAY MODE)
 * =============================================================================
 *
 * (start)
 *   Inicia ou retoma a partida quando o servidor está em modo de espera (before_kick_off).
 *   Equivalente a alterar o modo para play_on no momento do pontapé inicial.
 *
 *   Exemplo:
 *     (start)
 *
 * (change_mode <PLAY_MODE>)
 *   Força a transição imediata da partida para um estado/modo de jogo específico.
 *
 *   Parâmetros:
 *     <PLAY_MODE>: O identificador do modo de jogo desejado.
 *
 *   Exemplos:
 *     (change_mode play_on)
 *     (change_mode free_kick_l)
 *     (change_mode before_kick_off)
 *
 * =============================================================================
 * 3. MOVIMENTAÇÃO E POSICIONAMENTO (TELEPORTE)
 * =============================================================================
 *
 * (move (ball) <X> <Y> [<DIR> [<VX> <VY>]])
 *   Teletransporta a bola para uma coordenada cartesiana específica no campo.
 *
 *   Parâmetros:
 *     <X> <Y>: Coordenadas no campo (em metros, centro em 0 0).
 *     <DIR> (opcional): Ângulo/Direção.
 *     <VX> <VY> (opcional): Vetor de velocidade inicial nos eixos X e Y.
 *
 *   Exemplos:
 *     ; Coloca a bola no centro do campo
 *     (move (ball) 0 0)
 *
 *     ; Move a bola para a marca do pênalti com velocidade zero
 *     (move (ball) 36 0 0 0 0)
 *
 * (move (player <TEAM> <UNUM>) <X> <Y> [<DIR> [<VX> <VY>]])
 *   Teletransporta um jogador para uma posição cartesiana.
 *
 *   Parâmetros:
 *     <TEAM>: Nome do time (our, opp ou o nome exato registrado do time).
 *     <UNUM>: Número da camisa do jogador (1 a 11).
 *     <X> <Y>: Posição cartesiana no campo.
 *     <DIR> (opcional): Orientação do corpo em graus (relativo ao eixo X positivo).
 *     <VX> <VY> (opcional): Velocidade inicial do jogador.
 *
 *   Exemplos:
 *     ; Teletransporta o jogador 7 do nosso time para (10, -5) apontando para 0 graus
 *     (move (player our 7) 10 -5 0)
 *
 *     ; Posiciona o goleiro adversário (camisa 1) em (-45, 0)
 *     (move (player opp 1) -45 0)
 *
 * =============================================================================
 * 4. ALTERAÇÃO DE ATRIBUTOS E ESTADO DE JOGADORES
 * =============================================================================
 *
 * (change_player_type <TEAM> <UNUM> <TYPE_ID>)
 *   Altera o tipo heterogêneo de um jogador antes ou durante o treino.
 *
 *   Parâmetros:
 *     <TEAM>: Nome do time (our, opp ou nome exato).
 *     <UNUM>: Número do jogador.
 *     <TYPE_ID>: ID do tipo heterogêneo (0 é o jogador padrão/default; 1 a 17 são os perfis gerados pelo servidor).
 *
 *   Exemplos:
 *     (change_player_type our 9 3)
 *     (change_player_type opp 10 0)
 *
 * (stamina (player <TEAM> <UNUM>) <STAMINA> [<RECOVERY> [<EFFORT> [<STAMINA_CAP]]])
 *   Ajusta manualmente o nível e a capacidade de stamina de um jogador específico.
 *
 *   Parâmetros:
 *     <STAMINA>: Nível atual de stamina (valor padrão máximo de até 8000.0).
 *     <RECOVERY> (opcional): Taxa de recuperação de stamina.
 *     <EFFORT> (opcional): Fator de esforço do jogador.
 *     <STAMINA_CAP> (opcional): Capacidade máxima de stamina para a partida.
 *
 *   Exemplos:
 *     ; Restaura totalmente a stamina do jogador 11 do nosso time
 *     (stamina (player our 11) 8000.0)
 *
 *     ; Ajusta stamina, recuperação e esforço
 *     (stamina (player opp 5) 4000.0 1.0 1.0)
 *
 * (recover)
 *   Restaura instantaneamente a stamina, capacidade de recuperação e esforço de todos
 *   os jogadores em campo para os valores máximos.
 *
 *   Exemplo:
 *     (recover)
 *
 * (card (player <TEAM> <UNUM>) <yellow|red>)
 *   Aplica uma punição por cartão amarelo ou vermelho a um jogador.
 *
 *   Parâmetros:
 *     <yellow|red>: Tipo do cartão (yellow ou red).
 *
 *   Exemplos:
 *     (card (player opp 4) yellow)
 *     (card (player our 2) red)
 *
 * =============================================================================
 * 5. COMUNICAÇÃO COM OS AGENTES
 * =============================================================================
 *
 * (say <MESSAGE>)
 *   Transmite uma mensagem enviada pelo Trainer que pode ser ouvida por todos os
 *   agentes em campo (respeitando as regras e limites de áudio do servidor para o trainer).
 *
 *   Parâmetros:
 *     <MESSAGE>: String contendo a mensagem a ser transmitida.
 *
 *   Exemplos:
 *     (say "RESET_DRILL")
 *     (say "(free_form_instruction)")
 *
 * =============================================================================
 * 6. CONSULTAS DE ESTADO DO CAMPO
 * =============================================================================
 *
 * (check_ball)
 *   Solicita ao servidor a confirmação da posição e estado atual da bola no campo
 *   (se está em jogo, fora de campo, na rede, etc.). O servidor responde com uma
 *   mensagem (check_ball <TIME> <STATUS>).
 *
 *   Exemplo:
 *     (check_ball)
 *
 * (look)
 *   Solicita um relatório completo com as posições cartesianas e velocidades exatas
 *   de todos os jogadores e da bola no instante do ciclo atual.
 *
 *   Exemplo:
 *     (look)
 *
 * (team_names)
 *   Solicita ao servidor o nome dos dois times conectados no momento. O servidor
 *   responde com (team_names (left <NAME>) (right <NAME>)).
 *
 *   Exemplo:
 *     (team_names)
 *
 * =============================================================================
 * 7. CONFIGURAÇÕES DE FLUXO E SENSORES
 * =============================================================================
 *
 * (eye <on|off>)
 *   Ativa ou desativa a recepção de informações visuais do campo pelo próprio
 *   Trainer a cada ciclo (mensagem see_global).
 *
 *   Exemplos:
 *     (eye on)
 *     (eye off)
 *
 * (ear <on|off>)
 *   Ativa ou desativa a capacidade do Trainer de ouvir mensagens de áudio enviadas
 *   pelos agentes em campo.
 *
 *   Exemplo:
 *     (ear on)
 *
 * (compression <LEVEL>)
 *   Define o nível de compressão zlib para o tráfego de dados do servidor para o Trainer.
 *
 *   Parâmetros:
 *     <LEVEL>: Inteiro de 0 (sem compressão) a 9 (compressão máxima).
 *
 *   Exemplo:
 *     (compression 6)
 *
 * =============================================================================
 * 8. TABELA DE MODOS DE JOGO (PLAY MODES)
 * =============================================================================
 *
 * before_kick_off       - Antes do início da partida
 * play_on               - Jogo em andamento normal
 * time_over             - Partida finalizada
 * kick_off_l            - Pontapé inicial para o time da esquerda
 * kick_off_r            - Pontapé inicial para o time da direita
 * kick_in_l             - Arremesso lateral para o time da esquerda
 * kick_in_r             - Arremesso lateral para o time da direita
 * free_kick_l           - Tiro livre para o time da esquerda
 * free_kick_r           - Tiro livre para o time da direita
 * corner_kick_l         - Escanteio para o time da esquerda
 * corner_kick_r         - Escanteio para o time da direita
 * goal_kick_l           - Tiro de meta para o time da esquerda
 * goal_kick_r           - Tiro de meta para o time da direita
 * goal_l                - Gol marcado pelo time da esquerda
 * goal_r                - Gol marcado pelo time da direita
 * offside_l             - Impedimento marcado contra o time da esquerda
 * offside_r             - Impedimento marcado contra o time da direita
 * drop_ball             - Bola ao chão (árbitro)
 * penalty_setup_l       - Preparação para pênalti do time da esquerda
 * penalty_setup_r       - Preparação para pênalti do time da direita
 * penalty_ready_l       - Pênalti pronto para o time da esquerda
 * penalty_ready_r       - Pênalti pronto para o time da direita
 * penalty_taken_l       - Pênalti executado pelo time da esquerda
 * penalty_taken_r       - Pênalti executado pelo time da direita
 * penalty_miss_l        - Pênalti perdido pelo time da esquerda
 * penalty_miss_r        - Pênalti perdido pelo time da direita
 * penalty_score_l       - Pênalti convertido pelo time da esquerda
 * penalty_score_r       - Pênalti convertido pelo time da direita
 * penalty_onfield_l     - Pênalti em campo para o time da esquerda
 * penalty_onfield_r     - Pênalti em campo para o time da direita
 * after_goal_l          - Período após gol do time da esquerda
 * after_goal_r          - Período após gol do time da direita
 * back_pass_l           - Passe para trás do time da esquerda
 * back_pass_r           - Passe para trás do time da direita
 * indirect_free_kick_l  - Tiro livre indireto para o time da esquerda
 * indirect_free_kick_r  - Tiro livre indireto para o time da direita
 */

int main(int argc, char* argv[]) {

    /* -- Parsing de Possibilidades -- */

    // Criamos o parser do nosso binário
    cxxopts::Options options(
        "",
        "Executável do tipo ELF responsável por prover à RoboIME um gerenciamento do treinamento dos jogadores."
    );

    // Definimos as possibilidades
    options.add_options()
    (
        "s,speed",
        "Velocidade da Simulação",
        cxxopts::value<float>()
            ->default_value("1")
    )
    (
        "h,help",
        "Mostrar esta mensagem que está lida"
    );

    // A partir da matriz de possibilidades acima, realizamos o parsing
    cxxopts::ParseResult result = options.parse(argc, argv);

    // Realizamos algumas verificações
    if(result.count("help")) {
        std::cout << options.help() << "\n";
        return 0;
    }


    // Considerando que estamos treinando, estaremos em ambiente controlado por nós
    // Logo, localhost
    TrainerAgent trainer {"127.0.0.1", 6001, result["speed"].as<float>()};
    while(true) {

        if(trainer.run()) {
            return 0;
        }
    }

    return 0;
}