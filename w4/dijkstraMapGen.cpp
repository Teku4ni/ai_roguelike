#include "dijkstraMapGen.h"
#include "ecsTypes.h"
#include "dungeonUtils.h"
#include "math.h"

template<typename Callable>
static void query_dungeon_data(flecs::world &ecs, Callable c)
{
  static auto dungeonDataQuery = ecs.query<const DungeonData>();

  dungeonDataQuery.each(c);
}

template<typename Callable>
static void query_characters_positions(flecs::world &ecs, Callable c)
{
  static auto characterPositionQuery = ecs.query<const Position, const Team>();

  characterPositionQuery.each(c);
}

constexpr float invalid_tile_value = 1e5f;
constexpr float forbidden_tile_value = 1e6f;

static void init_tiles(std::vector<float> &map, const DungeonData &dd)
{
  map.resize(dd.width * dd.height);
  for (float &v : map)
    v = invalid_tile_value;
}

// scan version, could be implemented as Dijkstra version as well
static void process_dmap(std::vector<float> &map, const DungeonData &dd)
{
  bool done = false;
  auto getMapAt = [&](size_t x, size_t y, float def)
  {
    if (x < dd.width && y < dd.width && dd.tiles[y * dd.width + x] == dungeon::floor)
      return map[y * dd.width + x];
    return def;
  };
  auto getMinNei = [&](size_t x, size_t y)
  {
    float val = map[y * dd.width + x];
    val = std::min(val, getMapAt(x - 1, y + 0, val));
    val = std::min(val, getMapAt(x + 1, y + 0, val));
    val = std::min(val, getMapAt(x + 0, y - 1, val));
    val = std::min(val, getMapAt(x + 0, y + 1, val));
    return val;
  };
  while (!done)
  {
    done = true;
    for (size_t y = 0; y < dd.height; ++y)
      for (size_t x = 0; x < dd.width; ++x)
      {
        const size_t i = y * dd.width + x;
        if (dd.tiles[i] != dungeon::floor)
          continue;
        const float myVal = getMapAt(x, y, invalid_tile_value);
        const float minVal = getMinNei(x, y);
        if (minVal < myVal - 1.f)
        {
          map[i] = minVal + 1.f;
          done = false;
        }
      }
  }
}

void dmaps::gen_enemy_approach_map(flecs::world &ecs, std::vector<float> &map, int team)
{
  query_dungeon_data(ecs, [&](const DungeonData &dd)
  {
    init_tiles(map, dd);
    query_characters_positions(ecs, [&](const Position &pos, const Team &t)
    {
      if (t.team != team)
        map[pos.y * dd.width + pos.x] = 0.f;
    });
    process_dmap(map, dd);
  });
}

void dmaps::gen_enemy_flee_map(flecs::world &ecs, std::vector<float> &map, int team)
{
  gen_enemy_approach_map(ecs, map, team);
  for (float &v : map)
    if (v < invalid_tile_value)
      v *= -1.2f;
  query_dungeon_data(ecs, [&](const DungeonData &dd)
  {
    process_dmap(map, dd);
  });
}

void dmaps::gen_enemy_mage_map(flecs::world &ecs, std::vector<float> &map, float lowerBound, float upperBound, int team)
{
  query_dungeon_data(ecs, [&](const DungeonData &dd)
  {
    init_tiles(map, dd);
    query_characters_positions(ecs, [&](const Position &pos, const Team &t)
    {
      if (t.team != team)
      {
        int yStart = std::max(pos.y - int(ceil(upperBound)), 0);
        int yEnd = std::min(pos.y + int(ceil(upperBound)), int(dd.width) - 1);
        int xStart = std::max(pos.x - int(ceil(upperBound)), 0);
        int xEnd = std::min(pos.x + int(ceil(upperBound)), int(dd.height) - 1);
        for (int y = yStart; y < yEnd; ++y)
          for (int x = xStart; x < xEnd; ++x)
          {
            int posInMap = y * dd.width + x;
            if (dd.tiles[posInMap] != dungeon::floor)
              continue;
            if (dist_sq(pos, Position{x, y}) <= sqr(lowerBound))
              map[posInMap] = forbidden_tile_value;
            else if (dist_sq(pos, Position{x, y}) <= sqr(upperBound) && map[posInMap] == invalid_tile_value)
              map[posInMap] = 0;
          }
      }
    });
    process_dmap(map, dd);
  });
}

void dmaps::gen_hive_pack_map(flecs::world &ecs, std::vector<float> &map)
{
  static auto hiveQuery = ecs.query<const Position, const Hive>();
  query_dungeon_data(ecs, [&](const DungeonData &dd)
  {
    init_tiles(map, dd);
    hiveQuery.each([&](const Position &pos, const Hive &)
    {
      map[pos.y * dd.width + pos.x] = 0.f;
    });
    process_dmap(map, dd);
  });
}

void dmaps::gen_exploration_map(flecs::world& ecs, std::vector<float>& map)
{
  static auto explorationQuery = ecs.query<const ExplorationMap>();
  query_dungeon_data(ecs, [&](const DungeonData &dd)
  {
    init_tiles(map, dd);
    explorationQuery.each([&](const ExplorationMap &expMap)
    {
      for (int i = 0; i < map.size(); ++i)
      {
        if (dd.tiles[i] != dungeon::floor)
          continue;
        if (!expMap.explored[i])
          map[i] = 0.f;
      }
    });
    process_dmap(map, dd);
  });
}

