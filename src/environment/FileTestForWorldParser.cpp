#include "./Environment.hpp"

/* Para realizarmos testes gerais de algoritmo */
static constexpr std::array<std::string_view, 8> messages_from_server {
    // 0 - initialization_message_from_server
    "(server_param (audio_cut_dist 50)(auto_mode 0)(back_dash_rate 0.7)(back_passes 1)(ball_accel_max 2.7)(ball_decay 0.94)(ball_rand 0.05)(ball_size 0.085)(ball_speed_max 3)(ball_stuck_area 3)(ball_weight 0.2)(catch_ban_cycle 5)(catch_probability 1)(catchable_area_l 1.2)(catchable_area_w 1)(ckick_margin 1)(clang_advice_win 1)(clang_define_win 1)(clang_del_win 1)(clang_info_win 1)(clang_mess_delay 50)(clang_mess_per_cycle 1)(clang_meta_win 1)(clang_rule_win 1)(clang_win_size 300)(coach 0)(coach_port 6001)(coach_w_referee 0)(connect_wait 300)(control_radius 2)(dash_angle_step 1)(dash_power_rate 0.006)(drop_ball_time 100)(effort_dec 0.005)(effort_dec_thr 0.3)(effort_inc 0.01)(effort_inc_thr 0.6)(effort_init 1)(effort_min 0.6)(extra_half_time 100)(extra_stamina 50)(fixed_teamname_l \"\")(fixed_teamname_r \"\")(forbid_kick_off_offside 1)(foul_cycles 5)(foul_detect_probability 0.5)(foul_exponent 10)(free_kick_faults 1)(freeform_send_period 20)(freeform_wait_period 600)(fullstate_l 0)(fullstate_r 0)(game_log_compression 0)(game_log_dated 1)(game_log_dir \"./logs\")(game_log_fixed 0)(game_log_fixed_name \"rcssserver\")(game_log_version 6)(game_logging 1)(game_over_wait 100)(goal_width 14.02)(goalie_max_moves 2)(golden_goal 0)(half_time 300)(hear_decay 1)(hear_inc 1)(hear_max 1)(illegal_defense_dist_x 16.5)(illegal_defense_duration 20)(illegal_defense_number 0)(illegal_defense_width 40.32)(inertia_moment 5)(keepaway 0)(keepaway_length 20)(keepaway_log_dated 1)(keepaway_log_dir \"./\")(keepaway_log_fixed 0)(keepaway_log_fixed_name \"rcssserver\")(keepaway_logging 1)(keepaway_start -1)(keepaway_width 20)(kick_off_wait 100)(kick_power_rate 0.027)(kick_rand 0.1)(kick_rand_factor_l 1)(kick_rand_factor_r 1)(kickable_margin 0.7)(landmark_file \"~/.rcssserver-landmark.xml\")(log_date_format \"%Y%m%d%H%M%S-\")(log_times 0)(max_back_tackle_power 0)(max_catch_angle 90)(max_dash_angle 180)(max_dash_power 100)(max_goal_kicks 3)(max_tackle_power 100)(maxmoment 180)(maxneckang 90)(maxneckmoment 180)(maxpower 100)(min_catch_angle -90)(min_dash_angle -180)(min_dash_power 0)(minmoment -180)(minneckang -90)(minneckmoment -180)(minpower -100)(nr_extra_halfs 2)(nr_normal_halfs 2)(offside_active_area_size 2.5)(offside_kick_margin 9.15)(olcoach_port 6002)(old_coach_hear 0)(pen_allow_mult_kicks 1)(pen_before_setup_wait 10)(pen_coach_moves_players 1)(pen_dist_x 42.5)(pen_max_extra_kicks 5)(pen_max_goalie_dist_x 14)(pen_nr_kicks 5)(pen_random_winner 0)(pen_ready_wait 10)(pen_setup_wait 70)(pen_taken_wait 150)(penalty_shoot_outs 1)(player_accel_max 1)(player_decay 0.4)(player_rand 0.1)(player_size 0.3)(player_speed_max 1.05)(player_speed_max_min 0.75)(player_weight 60)(point_to_ban 5)(point_to_duration 20)(port 6000)(prand_factor_l 1)(prand_factor_r 1)(profile 0)(proper_goal_kicks 0)(quantize_step 0.1)(quantize_step_l 0.01)(record_messages 0)(recover_dec 0.002)(recover_dec_thr 0.3)(recover_init 1)(recover_min 0.5)(recv_step 10)(red_card_probability 0)(say_coach_cnt_max 128)(say_coach_msg_size 128)(say_msg_size 10)(send_comms 0)(send_step 150)(send_vi_step 100)(sense_body_step 100)(side_dash_rate 0.4)(simulator_step 100)(slow_down_factor 1)(slowness_on_top_for_left_team 1)(slowness_on_top_for_right_team 1)(stamina_capacity 130600)(stamina_inc_max 45)(stamina_max 8000)(start_goal_l 0)(start_goal_r 0)(stopped_ball_vel 0.01)(synch_micro_sleep 1)(synch_mode 0)(synch_offset 60)(synch_see_offset 0)(tackle_back_dist 0)(tackle_cycles 10)(tackle_dist 2)(tackle_exponent 6)(tackle_power_rate 0.027)(tackle_rand_factor 2)(tackle_width 1.25)(team_actuator_noise 0)(team_l_start \"\")(team_r_start \"\")(text_log_compression 0)(text_log_dated 1)(text_log_dir \"./logs\")(text_log_fixed 0)(text_log_fixed_name \"rcssserver\")(text_logging 1)(use_offside 1)(verbose 0)(visible_angle 90)(visible_distance 3)(wind_ang 0)(wind_dir 0)(wind_force 0)(wind_none 0)(wind_rand 0)(wind_random 0))",
    // 1 - initialization_message_from_server_to_inicialize_players
    "(player_param (allow_mult_default_type 0)(catchable_area_l_stretch_max 1.3)(catchable_area_l_stretch_min 1)(dash_power_rate_delta_max 0)(dash_power_rate_delta_min 0)(effort_max_delta_factor -0.004)(effort_min_delta_factor -0.004)(extra_stamina_delta_max 50)(extra_stamina_delta_min 0)(foul_detect_probability_delta_factor 0)(inertia_moment_delta_factor 25)(kick_power_rate_delta_max 0)(kick_power_rate_delta_min 0)(kick_rand_delta_factor 1)(kickable_margin_delta_max 0.1)(kickable_margin_delta_min -0.1)(new_dash_power_rate_delta_max 0.0008)(new_dash_power_rate_delta_min -0.0012)(new_stamina_inc_max_delta_factor -6000)(player_decay_delta_max 0.1)(player_decay_delta_min -0.1)(player_size_delta_factor -100)(player_speed_max_delta_max 0)(player_speed_max_delta_min 0)(player_types 18)(pt_max 1)(random_seed 1784289745)(stamina_inc_max_delta_factor 0)(subs_max 3))",
    // 2 - example_initialization_message_to_inicialize_players
    "(player_type (id 1)(player_speed_max 1.05)(stamina_inc_max 45.7268)(player_decay 0.334255)(inertia_moment 3.35638)(dash_power_rate 0.00587886)(player_size 0.3)(kickable_margin 0.613565)(kick_rand 0.0135647)(extra_stamina 73.5228)(effort_max 0.905909)(effort_min 0.505909)(kick_power_rate 0.027)(foul_detect_probability 0.5)(catchable_area_l_stretch 1.15827)(unum_far_length 20)(unum_too_far_length 40)(team_far_length 125.096)(team_too_far_length 125.096)(player_max_observation_length 125.096)(ball_vel_far_length 20)(ball_vel_too_far_length 40)(ball_max_observation_length 125.096)(flag_chg_far_length 20)(flag_chg_too_far_length 40)(flag_max_observation_length 125.096))",
    // 3 - ok_messages
    "(ok synch_see)(ok compression 3)",
    // 4 - sense_body_message
    "(sense_body 0 (view_mode high normal) (stamina 8000 1 130600) (speed 0 0) (head_angle 0) (kick 0) (dash 0) (turn 0) (say 0) (turn_neck 0) (catch 0) (move 0) (change_view 0) (change_focus 0) (arm (movable 0) (expires 0) (target 0 0) (count 0)) (focus (target none) (count 0)) (tackle (expires 0) (count 0)) (collision none) (foul (charged 0) (card none)) (focus_point 0 0))",
    // 5 - see_message
    "(see 0 ((p \"RoboIME\" 8 (goalie)) 20 -15 -0.4 0.7 35 -10) ((f r t 10) 12.3 0 0 0) ((f r t) 73 -28) ((f r b) 73 28) ((f g r b) 65.4 6) ((g r) 64.7 0) ((f g r t) 65.4 -6) ((f p r b) 52.5 23) ((f p r c) 48.4 0) ((f p r t) 52.5 -23) ((f t r 20) 50.9 -50) ((f t r 30) 57.4 -43) ((f t r 40) 65.4 -37) ((f t r 50) 73.7 -32) ((f b r 20) 50.9 50) ((f b r 30) 57.4 43) ((f b r 40) 65.4 37) ((f b r 50) 73.7 32) ((f r 0) 70.1 0) ((f r t 10) 70.8 -8) ((f r t 20) 73 -16) ((f r t 30) 75.9 -23) ((f r b 10) 70.8 8) ((f r b 20) 73 16) ((f r b 30) 75.9 23) ((b) 12.2 0 -0 0) ((l r) 64.7 90))",
    // 6 - hear_messages
    "(hear 150 18 \"exemplo_mensagem\")(hear 1250 referee goal_l_1)((hear 412 self \"go\")(hear 300 online_coach_left \"(do our_offside_line 35)\")",
    // 7 - Exceptions
    "(change_player_type 7 3)(warning no_subs_left)(error unknown_command)"
};

/**
Comando de compilação pelo diretório chefe
g++ -std=c++20 -g3 -O0 -fno-inline -march=native -pthread -Wall ./src/environment/FileTestForWorldParser.cpp -o ./src/bin/FileTestForWorldParser
Comando de execução pelo gdb
gdb --args ./src./bin/FileTestForWorldParser 5
*/
int main(int argc, char* argv[]) {

    if(argc < 2) {
        std::cout << "Introduza o número da mensagem que será testada!" << std::endl;
        return 1;
    }

    const std::string_view message = messages_from_server[std::stoi(argv[1])];

    Environment env {};
    env.wp.update_from_server(message, env);
    return 0;
}

