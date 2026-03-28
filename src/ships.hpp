#pragma once
#include <vector>
#include "ecs.hpp"
#include "input.hpp"

namespace battleship_bebop {
  enum class ShipTier {
    Light,
    Medium,
    Heavy
  };

  struct CombatStats {
    int hp;
    int max_hp;
    int damage_direct;
    int damage_indirect;
  };

  struct ActionState {
    bool is_attacking_this_tick;
    float target_x;
    float target_y;
  };

  struct ShipStats {
    ShipTier tier;
    int length;
    int ticks_per_move;
  };

  struct MovementState {
    int ticks_til_move;
    bool can_move;
    bool set_move;
  };

  CombatStats get_base_stats(ShipTier tier) {
    switch(tier) {
      case ShipTier::Heavy:  return {200, 200, 100, 50};
      case ShipTier::Medium: return {100, 100, 50, 25};
      case ShipTier::Light:  return {50, 50, 25, 0};
    }
    return {0, 0, 0, 0};
  }

  struct FleetRegistry {
    std::vector<ShipStats> stats;
    std::vector<MovementState> movement;
    std::vector<CombatStats> combat;
    std::vector<ActionState> actions;

    void init_ship(size_t entity_id, ShipTier tier, int length, int speed) {
      if (entity_id >= stats.size()) {
        stats.resize(entity_id + 100);
        movement.resize(entity_id + 100);
        combat.resize(entity_id + 100);
        actions.resize(entity_id + 100);
      }
      stats[entity_id] = {tier, length, speed};
      movement[entity_id] = {0, false, false};
      combat[entity_id] = get_base_stats(tier);
      actions[entity_id] = {false, 0.0f, 0.0f};
    }
  };

  //void fleet_movement_system(Registry& registry, FleetRegistry& fleet, Input& input);
}
