#pragma once

#include "stateMachine.h"

// states
State *create_sm_state(StateMachine *sm);
State *create_attack_enemy_state();
State *create_move_to_enemy_state();
State *create_move_to_player_state();
State *create_heal_player_state();
State *create_split_state();
State *create_move_to_pos_state(int x, int y);
State *create_eat_state();
State *create_sleep_state();
State *create_craft_state();
State *create_sell_state();
State *create_flee_from_enemy_state();
State *create_patrol_state(float patrol_dist);
State *create_nop_state();

// transitions
StateTransition *create_always_transition();
StateTransition *create_enemy_available_transition(float dist);
StateTransition *create_enemy_reachable_transition();
StateTransition *create_hitpoints_less_than_transition(float thres);
StateTransition *create_player_hitpoints_less_than_transition(float thres);
StateTransition *create_player_available_transition(float dist);
StateTransition *create_can_split_transition();
StateTransition *create_reach_pos_transition(int x, int y);
StateTransition *create_craftsman_sleepy_transition(float sleepThres);
StateTransition *create_craftsman_hungry_transition(float sleepThres, float hungerThres);
StateTransition *create_craftsman_happy_transition(float sleepThres, float hungerThres);
StateTransition *create_craftsman_crafting_transition(float time);
StateTransition *create_negate_transition(StateTransition *in);
StateTransition *create_and_transition(StateTransition *lhs, StateTransition *rhs);

template<typename T>
T sqr(T a) { return a * a; }

template<typename T, typename U>
static float dist_sq(const T &lhs, const U &rhs) { return float(sqr(lhs.x - rhs.x) + sqr(lhs.y - rhs.y)); }

template<typename T, typename U>
static float dist(const T &lhs, const U &rhs) { return sqrtf(dist_sq(lhs, rhs)); }
