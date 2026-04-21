#pragma once

#include <string_view>

namespace rcss {
    namespace server {
        // Distância máxima para audição de mensagens
        inline constexpr int audio_cut_dist = 50;

        // Define se o modo automático está ativo
        inline constexpr bool auto_mode = false;

        // Taxa de aceleração para dash para trás
        inline constexpr double back_dash_rate = 0.7;

        // Permissão para passes para trás
        inline constexpr int back_passes = 1;

        // Aceleração máxima da bola
        inline constexpr double ball_accel_max = 2.7;

        // Taxa de decaimento da velocidade da bola por ciclo
        inline constexpr double ball_decay = 0.94;

        // Fator de aleatoriedade no movimento da bola
        inline constexpr double ball_rand = 0.05;

        // Raio/tamanho da bola
        inline constexpr double ball_size = 0.085;

        // Velocidade máxima permitida para a bola
        inline constexpr double ball_speed_max = 3.0;

        // Área de colisão onde a bola é considerada "presa"
        inline constexpr double ball_stuck_area = 3.0;

        // Peso da bola (influencia inércia)
        inline constexpr double ball_weight = 0.2;

        // Ciclos de proibição após capturar a bola (goleiro)
        inline constexpr int catch_ban_cycle = 5;
        
        // Probabilidade de sucesso na captura da bola
        inline constexpr double catch_probability = 1.0;
        
        // Comprimento da área de captura do goleiro
        inline constexpr double catchable_area_l = 1.2;
        
        // Largura da área de captura do goleiro
        inline constexpr double catchable_area_w = 1.0;
        
        // Margem de chute para o comando kick
        inline constexpr double ckick_margin = 1.0;

        /* Há alguns parâmetros intimamente relacionados ao coach */

        // Raio de controle do jogador sobre a bola
        inline constexpr double control_radius = 2.0;
        
        // Passo angular para o comando dash
        inline constexpr double dash_angle_step = 1.0;
        
        // Multiplicador de potência para o dash
        inline constexpr double dash_power_rate = 0.006;
        
        // Tempo para o drop ball em caso de jogo parado
        inline constexpr int drop_ball_time = 100;
        
        // Taxa de decréscimo do esforço (effort)
        inline constexpr double effort_dec = 0.005;
        
        // Limiar para início do decréscimo de esforço
        inline constexpr double effort_dec_thr = 0.3;
        
        // Taxa de incremento do esforço
        inline constexpr double effort_inc = 0.01;
        
        // Limiar para início do incremento de esforço
        inline constexpr double effort_inc_thr = 0.6;
        
        // Valor inicial de esforço
        inline constexpr double effort_init = 1.0;
        
        // Valor mínimo de esforço
        inline constexpr double effort_min = 0.6;
        
        // Duração do tempo extra
        inline constexpr int extra_half_time = 100;
        
        // Stamina extra concedida em prorrogação
        inline constexpr double extra_stamina = 50.0;
        
        /* Há alguns parâmetros relativos à faltas */
        
        // Largura do gol
        inline constexpr double goal_width = 14.02;
        
        // Máximo de movimentos livres do goleiro
        inline constexpr int goalie_max_moves = 2;
        
        // Parâmetros de decaimento de audição
        inline constexpr double hear_decay = 1.0;
        inline constexpr double hear_inc = 1.0;
        inline constexpr double hear_max = 1.0;
        
        // Defesas ilegais (parâmetros de área e duração)
        inline constexpr double illegal_defense_dist_x = 16.5;
        inline constexpr int illegal_defense_duration = 20;
        inline constexpr int illegal_defense_number = 0;
        inline constexpr double illegal_defense_width = 40.32;
        
        // Momento de inércia do jogador
        inline constexpr double inertia_moment = 5.0;
        
        // Multiplicador de potência do chute
        inline constexpr double kick_power_rate = 0.027;
        
        // Fator de aleatoriedade no chute
        inline constexpr double kick_rand = 0.1;
        inline constexpr double kick_rand_factor_l = 1.0;
        inline constexpr double kick_rand_factor_r = 1.0;
        
        // Margem adicional para que a bola seja "chutável"
        inline constexpr double kickable_margin = 0.7;
        
        // Parâmetros de Tackle (Carrinho)
        inline constexpr double max_back_tackle_power = 0.0;
        inline constexpr double max_catch_angle = 90.0;
        inline constexpr double max_dash_angle = 180.0;
        inline constexpr double max_dash_power = 100.0;
        inline constexpr int max_goal_kicks = 3;
        inline constexpr double max_tackle_power = 100.0;
        
        // Limites físicos de movimento e pescoço
        inline constexpr double maxmoment = 180.0;
        inline constexpr double maxneckang = 90.0;
        inline constexpr double maxneckmoment = 180.0;
        inline constexpr double maxpower = 100.0;
        inline constexpr double min_catch_angle = -90.0;
        inline constexpr double min_dash_angle = -180.0;
        inline constexpr double min_dash_power = 0.0;
        inline constexpr double minmoment = -180.0;
        inline constexpr double minneckang = -90.0;
        inline constexpr double minneckmoment = -180.0;
        inline constexpr double minpower = -100.0;
        
        // Tamanho da área ativa de impedimento
        inline constexpr double offside_active_area_size = 2.5;
        
        // Margem para cobrança de impedimento
        inline constexpr double offside_kick_margin = 9.15;

        // Parâmetros de Disputa de Pênaltis
        inline constexpr bool pen_allow_mult_kicks = true;
        inline constexpr int pen_before_setup_wait = 10;
        inline constexpr bool pen_coach_moves_players = true;
        inline constexpr double pen_dist_x = 42.5;
        inline constexpr int pen_max_extra_kicks = 5;
        inline constexpr double pen_max_goalie_dist_x = 14.0;
        inline constexpr int pen_nr_kicks = 5;
        inline constexpr bool pen_random_winner = false;
        inline constexpr int pen_ready_wait = 10;
        inline constexpr int pen_setup_wait = 70;
        inline constexpr int pen_taken_wait = 150;
        inline constexpr bool penalty_shoot_outs = true;
        
        // Dinâmica do Jogador
        inline constexpr double player_accel_max = 1.0;
        inline constexpr double player_decay = 0.4;
        inline constexpr double player_rand = 0.1;
        inline constexpr double player_size = 0.3;
        inline constexpr double player_speed_max = 1.05;
        inline constexpr double player_speed_max_min = 0.75;
        inline constexpr double player_weight = 60.0;
        
        // Etapas de quantização de dados (sensores)
        inline constexpr double quantize_step = 0.1;
        inline constexpr double quantize_step_l = 0.01;
        
        // Recuperação de Stamina
        inline constexpr bool record_messages = false;
        inline constexpr double recover_dec = 0.002;
        inline constexpr double recover_dec_thr = 0.3;
        inline constexpr double recover_init = 1.0;
        inline constexpr double recover_min = 0.5;
        
        // Passos de simulação (em milissegundos)
        inline constexpr int recv_step = 10;
        inline constexpr double red_card_probability = 0.0;
        inline constexpr int say_coach_cnt_max = 128;
        inline constexpr int say_coach_msg_size = 128;
        inline constexpr int say_msg_size = 10;
        inline constexpr bool send_comms = false;
        inline constexpr int send_step = 150;
        inline constexpr int send_vi_step = 100;
        inline constexpr int sense_body_step = 100;
        
        // Taxa de dash lateral e passos de simulação
        inline constexpr double side_dash_rate = 0.4;
        inline constexpr int simulator_step = 100;
        inline constexpr double slow_down_factor = 1.0;
        inline constexpr double slowness_on_top_for_left_team = 1.0;
        inline constexpr double slowness_on_top_for_right_team = 1.0;
        
        // Parâmetros de Stamina (Energia)
        inline constexpr double stamina_capacity = 130600.0;
        inline constexpr double stamina_inc_max = 45.0;
        inline constexpr double stamina_max = 8000.0;
        
        // Velocidade considerada como bola parada
        inline constexpr double stopped_ball_vel = 0.01;
        
        // Detalhes de Tackle
        inline constexpr double tackle_back_dist = 0.0;
        inline constexpr int tackle_cycles = 10;
        inline constexpr double tackle_dist = 2.0;
        inline constexpr double tackle_exponent = 6.0;
        inline constexpr double tackle_power_rate = 0.027;
        inline constexpr double tackle_rand_factor = 2.0;
        inline constexpr double tackle_width = 1.25;
        
        // Visibilidade e Vento
        inline constexpr bool use_offside = true;
        inline constexpr bool verbose = false;
        inline constexpr double visible_angle = 90.0;
        inline constexpr double visible_distance = 3.0;
        inline constexpr double wind_ang = 0.0;
        inline constexpr double wind_dir = 0.0;
        inline constexpr double wind_force = 0.0;
        inline constexpr double wind_none = 0.0;
        inline constexpr double wind_rand = 0.0;
        inline constexpr double wind_random = 0.0;
        
    } // namespace server
    
    namespace player {
    
        // Limites de variação da área de captura (stretch)
        inline constexpr double catchable_area_l_stretch_max = 1.3;
        inline constexpr double catchable_area_l_stretch_min = 1.0;
        
        // Fatores de variação de esforço máximo e mínimo
        inline constexpr double effort_max_delta_factor = -0.004;
        inline constexpr double effort_min_delta_factor = -0.004;
        
        // Fator de variação para o momento de inércia
        inline constexpr double inertia_moment_delta_factor = 25.0;
        
        // Variação de taxa de potência de chute
        inline constexpr double kick_power_rate_delta_max = 0.0;
        inline constexpr double kick_power_rate_delta_min = 0.0;
        
        // Fator de variação de aleatoriedade no chute
        inline constexpr double kick_rand_delta_factor = 1.0;
        
        // Variação da margem de chute
        inline constexpr double kickable_margin_delta_max = 0.1;
        inline constexpr double kickable_margin_delta_min = -0.1;
        
        // Variações para a nova taxa de potência de dash
        inline constexpr double new_dash_power_rate_delta_max = 0.0008;
        inline constexpr double new_dash_power_rate_delta_min = -0.0012;
        
        // Fator de variação para incremento de stamina
        inline constexpr double new_stamina_inc_max_delta_factor = -6000.0;
        
        // Variação de decaimento do jogador
        inline constexpr double player_decay_delta_max = 0.1;
        inline constexpr double player_decay_delta_min = -0.1;
        
        // Variação de velocidade máxima do jogador
        inline constexpr double player_speed_max_delta_max = 0.0;
        inline constexpr double player_speed_max_delta_min = 0.0;
        
        // Parâmetros de tipos e aleatoriedade
        inline constexpr int pt_max = 1;
        inline constexpr long random_seed = 1776538395L;
        inline constexpr double stamina_inc_max_delta_factor = 0.0;
        
        // Máximo de substituições permitidas
        inline constexpr int subs_max = 3;
        
    } // namespace player
} // namespace rcss