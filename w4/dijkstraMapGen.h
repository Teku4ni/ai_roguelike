#pragma once
#include <vector>
#include <flecs.h>

namespace dmaps
{
  void gen_enemy_approach_map(flecs::world &ecs, std::vector<float> &map, int team);
  void gen_enemy_flee_map(flecs::world &ecs, std::vector<float> &map, int team);
  void gen_enemy_mage_map(flecs::world& ecs, std::vector<float>& map, float lowerBound, float upperBound, int team);
  void gen_hive_pack_map(flecs::world &ecs, std::vector<float> &map);
  void gen_exploration_map(flecs::world& ecs, std::vector<float>& map);
};

