#pragma once
#include <vector>
#include <algorithm>
#include "ecs.hpp"
#include "ships.hpp"

namespace battleship_bebop {
  struct EnemyTile {
    GridPos tile_pos;
    bool active;
  };

  struct EnemyRegistry {
    std::vector<ShipStats> stats;
    std::vector<MovementState> movement;
    std::vector<CombatStats> combat;
    std::vector<ActionState> actions;
    std::vector<EnemyTile> tiles;

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

    void sort_map_tiles() {
      if (tiles.size() < 1)
        return;

      std::sort(tiles.begin(), tiles.end(), [](const EnemyTile& tile_a, const EnemyTile& tile_b) {
        if (tile_a.tile_pos.x < tile_b.tile_pos.x)
          return true;

        if (tile_a.tile_pos.x == tile_b.tile_pos.x)
          return tile_a.tile_pos.y < tile_b.tile_pos.y;

        return false;
      });
    }

    void init_tile(size_t entity_id, int pos_x, int pos_y) {
      if (entity_id >= tiles.size()) {
        tiles.resize(entity_id + 100);
      }
      tiles[entity_id] = {{pos_x, pos_y}, false};
    }

    inline bool is_tile_occupied(int target_x, int target_y) {
      for (size_t i = 0; i < stats.size(); i++) {
        if (stats[i].length == 0) continue;
        
        auto tiles = get_ship_tiles(movement[i].current_grid_pos, 
                                    stats[i].length, 
                                    movement[i].facing_direction);
        for (const auto& tile : tiles) {
          if (tile.x == target_x && tile.y == target_y) return true;
        }
      }

      return false;
    }
  };
}
