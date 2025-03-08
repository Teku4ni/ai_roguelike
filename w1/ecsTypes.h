#pragma once

struct Position;
struct MovePos;

struct MovePos
{
  int x = 0;
  int y = 0;

  MovePos &operator=(const Position &rhs);
};

struct Position
{
  int x = 0;
  int y = 0;

  Position &operator=(const MovePos &rhs);
};

inline Position &Position::operator=(const MovePos &rhs)
{
  x = rhs.x;
  y = rhs.y;
  return *this;
}

inline MovePos &MovePos::operator=(const Position &rhs)
{
  x = rhs.x;
  y = rhs.y;
  return *this;
}

inline bool operator==(const Position &lhs, const Position &rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }
inline bool operator==(const Position &lhs, const MovePos &rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }
inline bool operator==(const MovePos &lhs, const MovePos &rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }
inline bool operator==(const MovePos &lhs, const Position &rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }


struct PatrolPos
{
  int x = 0;
  int y = 0;
};

struct Hitpoints
{
  float hitpoints = 10.f;
};

enum Actions
{
  EA_NOP = 0,
  EA_MOVE_START,
  EA_MOVE_LEFT = EA_MOVE_START,
  EA_MOVE_RIGHT,
  EA_MOVE_DOWN,
  EA_MOVE_UP,
  EA_MOVE_END,
  EA_ATTACK,
  EA_HEAL,
  EA_SPLIT,
  EA_EAT,
  EA_SLEEP,
  EA_CRAFT,
  EA_SELL,
  EA_NUM
};

struct Action
{
  int action = 0;
};

struct NumActions
{
  int numActions = 1;
  int curActions = 0;
};

struct MeleeDamage
{
  float damage = 2.f;
};

struct RangedAttack
{
  float damage = 15.f;
  float range = 5.f;
};

struct HealPickup
{
  float amount = 0.f;
};

struct Heal
{
  float amount = 0.f;
  float range = 1.f;
  int cooldown = 10;
  int curCooldown = 10;
};

struct CraftsmanStatuses
{
  int hunger = 0;
  int sleepiness = 0;
  int craftTimeRemaining = 0;
};

struct CraftsmanConsts
{
  Position eatPos = {-5, -5};
  Position sleepPos = {-5, -7};
  Position craftPos = {-7, -5};
  Position sellPos = {-7, -7};
  int maxCraftTime = 3;
};

struct PowerupAmount
{
  float amount = 0.f;
};

struct PlayerInput
{
  bool left = false;
  bool right = false;
  bool up = false;
  bool down = false;
};

struct Symbol
{
  char symb;
};

struct IsPlayer {};

struct IsSlime
{
  bool canSplit = true;
};

struct Team
{
  int team = 0;
};

struct TextureSource {};

