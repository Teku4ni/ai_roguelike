#include "aiLibrary.h"
#include <flecs.h>
#include "ecsTypes.h"
#include "raylib.h"
#include <cfloat>
#include <cmath>

class SMState : public State
{
  StateMachine *sm; // we own it
public:
  SMState(StateMachine *sm) : sm(sm) {}

  ~SMState() {delete sm;}

  void enter() const override {}
  void exit() const override {}
  void act(float dt, flecs::world &ecs, flecs::entity entity) const override
  {
    sm->act(dt, ecs, entity);
  }
};

class AttackEnemyState : public State
{
public:
  void enter() const override {}
  void exit() const override {}
  void act(float/* dt*/, flecs::world &/*ecs*/, flecs::entity entity) const override 
  {
    entity.insert([&](Action &a)
    {
      a.action = EA_ATTACK;
    });
  }
};

class SplitState : public State
{
public:
  void enter() const override {}
  void exit() const override {}
  void act(float/* dt*/, flecs::world &/*ecs*/, flecs::entity entity) const override
  {
    entity.insert([](Action &a) {
      a.action = EA_SPLIT;
      });
  }
};

class HealPlayerState : public State
{
public:
  void enter() const override {}
  void exit() const override {}
  void act(float/* dt*/, flecs::world &/*ecs*/, flecs::entity entity) const override
  {
    entity.insert([](Action& a)
    {
      a.action = EA_HEAL;
    });
  }
};

template<typename T, typename U>
static int move_towards(const T &from, const U &to)
{
  int deltaX = to.x - from.x;
  int deltaY = to.y - from.y;
  if (abs(deltaX) > abs(deltaY))
    return deltaX > 0 ? EA_MOVE_RIGHT : EA_MOVE_LEFT;
  return deltaY < 0 ? EA_MOVE_UP : EA_MOVE_DOWN;
}

static int inverse_move(int move)
{
  return move == EA_MOVE_LEFT ? EA_MOVE_RIGHT :
         move == EA_MOVE_RIGHT ? EA_MOVE_LEFT :
         move == EA_MOVE_UP ? EA_MOVE_DOWN :
         move == EA_MOVE_DOWN ? EA_MOVE_UP : move;
}


template<typename Callable>
static void on_closest_enemy_pos(flecs::world &ecs, flecs::entity entity, Callable c)
{
  static auto enemiesQuery = ecs.query<const Position, const Team>();
  entity.insert([&](const Position &pos, const Team &t, Action &a)
  {
    flecs::entity closestEnemy;
    float closestDist = FLT_MAX;
    Position closestPos;
    enemiesQuery.each([&](flecs::entity enemy, const Position &epos, const Team &et)
    {
      if (t.team == et.team)
        return;
      float curDist = dist(epos, pos);
      if (curDist < closestDist)
      {
        closestDist = curDist;
        closestPos = epos;
        closestEnemy = enemy;
      }
    });
    if (ecs.is_valid(closestEnemy))
      c(a, pos, closestPos);
  });
}

class MoveToEnemyState : public State
{
public:
  void enter() const override {}
  void exit() const override {}
  void act(float/* dt*/, flecs::world &ecs, flecs::entity entity) const override
  {
    on_closest_enemy_pos(ecs, entity, [&](Action &a, const Position &pos, const Position &enemy_pos)
    {
      a.action = move_towards(pos, enemy_pos);
    });
  }
};

class FleeFromEnemyState : public State
{
public:
  FleeFromEnemyState() {}
  void enter() const override {}
  void exit() const override {}
  void act(float/* dt*/, flecs::world &ecs, flecs::entity entity) const override
  {
    on_closest_enemy_pos(ecs, entity, [&](Action &a, const Position &pos, const Position &enemy_pos)
    {
      a.action = inverse_move(move_towards(pos, enemy_pos));
    });
  }
};

class MoveToPlayerState : public State
{
public:
  void enter() const override {}
  void exit() const override {}
  void act(float/* dt*/, flecs::world &ecs, flecs::entity entity) const override
  {
    static auto findPlayer = ecs.query<const IsPlayer, const Position>();
    entity.insert([&](Action &a, const Position &pos)
    {
      findPlayer.each([&](const IsPlayer, const Position &ppos)
      {
        a.action = move_towards(pos, ppos);
      });
    });
  }
};

class MoveToPosState : public State
{
  int x, y;
public:
  MoveToPosState(int x, int y) : x(x), y(y) {}
  void enter() const override {}
  void exit() const override {}
  void act(float/* dt*/, flecs::world &ecs, flecs::entity entity) const override
  {
    entity.insert([&](Action &a, const Position &pos)
    {
      a.action = move_towards(pos, Position{x, y});
    });
  }
};

class EatState : public State
{
public:
  void enter() const override {}
  void exit() const override {}
  void act(float/* dt*/, flecs::world&/*ecs*/, flecs::entity entity) const override
  {
    entity.insert([](Action &a)
    {
      a.action = EA_EAT;
    });
  }
};

class SleepState : public State
{
public:
  void enter() const override {}
  void exit() const override {}
  void act(float/* dt*/, flecs::world&/*ecs*/, flecs::entity entity) const override
  {
    entity.insert([](Action &a)
    {
      a.action = EA_SLEEP;
    });
  }
};

class CraftState : public State
{
public:
  void enter() const override {}
  void exit() const override {}
  void act(float/* dt*/, flecs::world&/*ecs*/, flecs::entity entity) const override
  {
    entity.insert([](Action &a)
    {
      a.action = EA_CRAFT;
    });
  }
};

class SellState : public State
{
public:
  void enter() const override {}
  void exit() const override {}
  void act(float/* dt*/, flecs::world&/*ecs*/, flecs::entity entity) const override
  {
    entity.insert([](Action &a)
    {
      a.action = EA_SELL;
    });
  }
};

class PatrolState : public State
{
  float patrolDist;
public:
  PatrolState(float dist) : patrolDist(dist) {}
  void enter() const override {}
  void exit() const override {}
  void act(float/* dt*/, flecs::world &ecs, flecs::entity entity) const override
  {
    entity.insert([&](const Position &pos, const PatrolPos &ppos, Action &a)
    {
      if (dist(pos, ppos) > patrolDist)
        a.action = move_towards(pos, ppos); // do a recovery walk
      else
      {
        // do a random walk
        a.action = GetRandomValue(EA_MOVE_START, EA_MOVE_END - 1);
      }
    });
  }
};

class NopState : public State
{
public:
  void enter() const override {}
  void exit() const override {}
  void act(float/* dt*/, flecs::world &ecs, flecs::entity entity) const override {}
};

class AlwaysTransition : public StateTransition
{
public:
  bool isAvailable(flecs::world &ecs, flecs::entity entity) const override
  {
    return true;
  }
};

class EnemyAvailableTransition : public StateTransition
{
  float triggerDist;
public:
  EnemyAvailableTransition(float in_dist) : triggerDist(in_dist) {}
  bool isAvailable(flecs::world &ecs, flecs::entity entity) const override
  {
    static auto enemiesQuery = ecs.query<const Position, const Team>();
    bool enemiesFound = false;
    entity.get([&](const Position &pos, const Team &t)
    {
      enemiesQuery.each([&](flecs::entity enemy, const Position &epos, const Team &et)
      {
        if (t.team == et.team)
          return;
        float curDist = dist(epos, pos);
        enemiesFound |= curDist <= triggerDist;
      });
    });
    return enemiesFound;
  }
};

class PlayerAvailableTransition : public StateTransition
{
  float triggerDist;
public:
  PlayerAvailableTransition(float in_dist) : triggerDist(in_dist) {}
  bool isAvailable(flecs::world &ecs, flecs::entity entity) const override
  {
    static auto playerQuery = ecs.query<const IsPlayer, const Position, const Team>();
    bool playerFound = false;
    entity.get([&](const Position &pos, const Team &t)
    {
      playerQuery.each([&](flecs::entity player, const IsPlayer, const Position &ppos, const Team &pt)
      {
        if (t.team != pt.team)
          return;
        float curDist = dist(ppos, pos);
        playerFound |= curDist <= triggerDist;
      });
    });
    return playerFound;
  }
};

class HitpointsLessThanTransition : public StateTransition
{
  float threshold;
public:
  HitpointsLessThanTransition(float in_thres) : threshold(in_thres) {}
  bool isAvailable(flecs::world &ecs, flecs::entity entity) const override
  {
    bool hitpointsThresholdReached = false;
    entity.get([&](const Hitpoints &hp)
    {
      hitpointsThresholdReached |= hp.hitpoints < threshold;
    });
    return hitpointsThresholdReached;
  }
};

class PlayerHitpointsLessThanTransition : public StateTransition
{
  float threshold;
public:
  PlayerHitpointsLessThanTransition(float in_thres) : threshold(in_thres) {}
  bool isAvailable(flecs::world &ecs, flecs::entity entity) const override
  {
    bool playerHitpointsThresholdReached = false;
    static auto findPlayer = ecs.query<const IsPlayer, const Hitpoints>();
    findPlayer.each([&](const IsPlayer, const Hitpoints &hp)
    {
        playerHitpointsThresholdReached |= hp.hitpoints < threshold;
    });
    return playerHitpointsThresholdReached;
  }
};

class CanSplitTransition : public StateTransition
{
public:
  bool isAvailable(flecs::world &ecs, flecs::entity entity) const override
  {
    bool canSplit = false;
    static auto findSlime = ecs.query<const IsSlime>();
    findSlime.each([&](const IsSlime &slime)
    {
      canSplit |= slime.canSplit;
    });
    return canSplit;
  }
};

class ReachPosTransition : public StateTransition
{
  int x, y;
public:
  ReachPosTransition(int x, int y) : x(x), y(y) {};
  bool isAvailable(flecs::world &ecs, flecs::entity entity) const override
  {
    bool reached = false;
    entity.get([&](const Position &pos, const Team& t)
    {
      reached = (pos == Position{x, y});
    });
    return reached;
  }
};

class CraftsmanHungryTransition : public StateTransition
{
  float sleepinessThreshold, hungerThreshold;
public:
  CraftsmanHungryTransition(float thres1, float thres2) : sleepinessThreshold(thres1), hungerThreshold(thres2) {}
  bool isAvailable(flecs::world &ecs, flecs::entity entity) const override
  {
    bool hungry = false;
    static auto findCraftsman = ecs.query<const CraftsmanStatuses>();
    findCraftsman.each([&](const CraftsmanStatuses &statuses)
    {
      hungry |= statuses.sleepiness <= sleepinessThreshold && statuses.hunger > hungerThreshold;
    });
    return hungry;
  }
};

class CraftsmanSleepyTransition : public StateTransition
{
  float threshold;
public:
  CraftsmanSleepyTransition(float thres) : threshold(thres) {}
  bool isAvailable(flecs::world &ecs, flecs::entity entity) const override
  {
    bool sleepy = false;
    static auto findCraftsman = ecs.query<const CraftsmanStatuses>();
    findCraftsman.each([&](const CraftsmanStatuses &statuses)
    {
      sleepy |= statuses.sleepiness > threshold;
    });
    return sleepy;
  }
};

class CraftsmanHappyTransition : public StateTransition
{
  float sleepinessThreshold, hungerThreshold;
public:
  CraftsmanHappyTransition(float thres1, float thres2) : sleepinessThreshold(thres1), hungerThreshold(thres2) {}
  bool isAvailable(flecs::world &ecs, flecs::entity entity) const override
  {
    bool happy = false;
    static auto findCraftsman = ecs.query<const CraftsmanStatuses>();
    findCraftsman.each([&](const CraftsmanStatuses& statuses)
    {
      happy |= statuses.hunger <= hungerThreshold && statuses.sleepiness <= sleepinessThreshold;
    });
    return happy;
  }
};

class CraftsmanCraftingTransition : public StateTransition
{
  float time;
public:
  CraftsmanCraftingTransition(float time) : time(time) {}
  bool isAvailable(flecs::world& ecs, flecs::entity entity) const override
  {
    bool crafting = true;
    static auto findCraftsman = ecs.query<const CraftsmanStatuses>();
    findCraftsman.each([&](const CraftsmanStatuses& statuses)
    {
      crafting &= statuses.craftTimeRemaining > 0;
    });
    return crafting;
  }
};

class EnemyReachableTransition : public StateTransition
{
public:
  bool isAvailable(flecs::world &ecs, flecs::entity entity) const override
  {
    return false;
  }
};

class NegateTransition : public StateTransition
{
  const StateTransition *transition; // we own it
public:
  NegateTransition(const StateTransition *in_trans) : transition(in_trans) {}
  ~NegateTransition() override { delete transition; }

  bool isAvailable(flecs::world &ecs, flecs::entity entity) const override
  {
    return !transition->isAvailable(ecs, entity);
  }
};

class AndTransition : public StateTransition
{
  const StateTransition *lhs; // we own it
  const StateTransition *rhs; // we own it
public:
  AndTransition(const StateTransition *in_lhs, const StateTransition *in_rhs) : lhs(in_lhs), rhs(in_rhs) {}
  ~AndTransition() override
  {
    delete lhs;
    delete rhs;
  }

  bool isAvailable(flecs::world &ecs, flecs::entity entity) const override
  {
    return lhs->isAvailable(ecs, entity) && rhs->isAvailable(ecs, entity);
  }
};


// states
State *create_sm_state(StateMachine *sm)
{
  return new SMState(sm);
}
State *create_attack_enemy_state()
{
  return new AttackEnemyState();
}
State *create_move_to_enemy_state()
{
  return new MoveToEnemyState();
}

State *create_split_state()
{
  return new SplitState();
}

State *create_move_to_player_state()
{
  return new MoveToPlayerState();
}
State *create_heal_player_state()
{
  return new HealPlayerState();
}

State *create_move_to_pos_state(int x, int y)
{
  return new MoveToPosState(x, y);
}

State *create_craft_state()
{
  return new CraftState();
}

State *create_sell_state()
{
  return new SellState();
}

State *create_eat_state()
{
  return new EatState();
}

State *create_sleep_state()
{
  return new SleepState();
}

State *create_flee_from_enemy_state()
{
  return new FleeFromEnemyState();
}


State *create_patrol_state(float patrol_dist)
{
  return new PatrolState(patrol_dist);
}

State *create_nop_state()
{
  return new NopState();
}

// transitions
StateTransition *create_always_transition()
{
  return new AlwaysTransition();
}

StateTransition *create_enemy_available_transition(float dist)
{
  return new EnemyAvailableTransition(dist);
}

StateTransition *create_enemy_reachable_transition()
{
  return new EnemyReachableTransition();
}

StateTransition *create_hitpoints_less_than_transition(float thres)
{
  return new HitpointsLessThanTransition(thres);
}

StateTransition *create_player_hitpoints_less_than_transition(float thres)
{
  return new PlayerHitpointsLessThanTransition(thres);
}

StateTransition *create_player_available_transition(float thres)
{
  return new PlayerAvailableTransition(thres);
}

StateTransition *create_can_split_transition()
{
  return new CanSplitTransition();
}

StateTransition *create_reach_pos_transition(int x, int y)
{
  return new ReachPosTransition(x, y);
}

StateTransition *create_craftsman_hungry_transition(float sleepThres, float hungerThres)
{
  return new CraftsmanHungryTransition(sleepThres, hungerThres);
}

StateTransition *create_craftsman_sleepy_transition(float thres)
{
  return new CraftsmanSleepyTransition(thres);
}

StateTransition *create_craftsman_happy_transition(float sleepThres, float hungerThres)
{
  return new CraftsmanHappyTransition(sleepThres, hungerThres);
}

StateTransition *create_craftsman_crafting_transition(float time)
{
  return new CraftsmanCraftingTransition(time);
}

StateTransition *create_negate_transition(StateTransition *in)
{
  return new NegateTransition(in);
}
StateTransition *create_and_transition(StateTransition *lhs, StateTransition *rhs)
{
  return new AndTransition(lhs, rhs);
}

