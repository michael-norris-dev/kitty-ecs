#pragma once
#include <vector>
#include "ecs.hpp"
#include "input.hpp"

enum class ShipTier {
  Light,
  Medium,
  Heavy
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

struct FleetRegistry {
  std::vector<ShipStats> stats;
  std::vector<MovementState> movement;

  void init_ship(size_t entity_id, ShipTier tier, int length, int speed) {
    if (entity_id >= stats.size()) {
      stats.resize(entity_id + 100);
      movement.resize(entity_id + 100);
    }
    stats[entity_id] = {tier, length, speed};
    movement[entity_id] = {0, false, false};
  }
};

//void fleet_movement_system(Registry& registry, FleetRegistry& fleet, Input& input);
