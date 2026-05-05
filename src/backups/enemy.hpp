#pragma once
#include <vector>
#include <algorithm>
#include "ecs.hpp"
#include "ships.hpp"

namespace battleship_bebop {
  struct EnemyTile {
    GridPos tile_pos;
    bool active;
    bool is_tile = false;
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
      movement[entity_id] = {0, true, false, {0, 0}, 1};
      movement[entity_id].is_rotating = false;
      movement[entity_id].rot_direction = 1;
      combat[entity_id] = get_base_stats(tier);
      actions[entity_id] = {false, 0.0f, 0.0f};
    }

    void spawn_ship(kitty_ecs::Registry& registry, ShipTier tier, 
        int ship_length, int ship_speed, float tile_size, int board_x, int board_y) {
      std::random_device rd;
      std::mt19937 gen(rd());

      std::uniform_int_distribution<> dis_x(0, board_x - 1);
      std::uniform_int_distribution<> dis_y(0, (board_y / 2) - 1);
      std::uniform_int_distribution<> dis_dir(1, 4);

      bool valid_spawn = false;
      int spawn_x, spawn_y, spawn_dir;

      while (!valid_spawn) {
        spawn_x = dis_x(gen);
        spawn_y = dis_y(gen);
        //spawn_dir = 1;
        spawn_dir = dis_dir(gen);

        valid_spawn = true;

        auto spawn_tiles = get_ship_tiles({spawn_x, spawn_y}, ship_length, spawn_dir, spawn_dir);

        for (const auto& tile : spawn_tiles) {
          if (tile.x < 1 || tile.x >= board_x - 1 || 
              tile.y >= (board_y / 2) || tile.y < 1) {
            valid_spawn = false;
            continue;
          }

          if (is_tile_occupied(-1, tile.x, tile.y)) {
            valid_spawn = false;
            continue;
          }
        }
      }

      size_t ship_id = registry.create_entity();
      init_ship(ship_id, tier, ship_length, ship_speed);
      movement[ship_id].current_grid_pos = {spawn_x, spawn_y};
      movement[ship_id].facing_direction = spawn_dir;
      movement[ship_id].rot_direction = spawn_dir;
      float tile_loc = -2.0f / board_y;
      float shift_factor_x = 1.0f - ((tile_size * 0.5f) / 10.0f);
      float shift_factor_y = 1.0f - (tile_size / 10.0f);

      float world_x = shift_factor_x + (spawn_x * (tile_loc * 0.5f));
      float world_y = shift_factor_y + (spawn_y * tile_loc);
      float radians = 0.0f;
      switch (spawn_dir) {
        case 1: break;
        case 2: radians = (3.0f * kitty_ecs::PI) / 2.0f; break;
        case 3: radians = kitty_ecs::PI; break;
        case 4: radians = kitty_ecs::PI / 2.0f; break;
      }
      registry.transforms[ship_id] = {world_x, world_y, 0.0f, 0.0f, radians, 1};
    }

    void init_tile(size_t entity_id, int pos_x, int pos_y) {
      if (entity_id >= tiles.size()) {
        tiles.resize(entity_id + 100);
      }
      tiles[entity_id] = {{pos_x, pos_y}, false, true};
    }

    inline bool is_tile_occupied(int ship_id, int target_x, int target_y) {
      for (size_t i = 0; i < stats.size(); i++) {
        if (stats[i].length == 0 || i == ship_id) continue; 

        auto tiles = get_ship_tiles(movement[i].current_grid_pos, 
            stats[i].length, 
            movement[i].facing_direction, 
            movement[i].rot_direction);
        for (const auto& tile : tiles) {
          if (tile.x == target_x && tile.y == target_y) return true;
        }
      }

      return false;
    }

    std::deque<GridPos> calculate_path(GridPos start, GridPos target, 
        int ship_length, int start_dir, int board_x, int board_y, int ship_id) {
      std::deque<GridPos> final_path;
      int max_x = board_x;
      int max_y = board_y;

      std::vector<std::vector<std::vector<int>>> g_costs(max_x, 
          std::vector<std::vector<int>>(max_y, std::vector<int>(5, 999999)));

      std::vector<std::vector<std::vector<ParentTracker>>> parents(max_x, 
          std::vector<std::vector<ParentTracker>>(max_y, std::vector<ParentTracker>(5)));

      std::priority_queue<PathNode, std::vector<PathNode>, std::greater<PathNode>> open_list;

      g_costs[start.x][start.y][start_dir] = 0;
      int start_h = std::abs(target.x - start.x) + std::abs(target.y - start.y);
      open_list.push({start, start_dir, start_dir, start_h});

      bool found_path = false;
      int end_dir = start_dir;

      while (!open_list.empty()) {
        PathNode current = open_list.top();
        open_list.pop();

        if (current.pos.x == target.x && current.pos.y == target.y) {
          found_path = true;
          end_dir = current.facing_dir;
          break; 
        }

        for (int new_dir = 1; new_dir <= 4; new_dir++) {
          GridPos next_pos = current.pos;

          if (new_dir == 1) next_pos.y += 1;
          else if (new_dir == 2) next_pos.x += 1;
          else if (new_dir == 3) next_pos.y -= 1;
          else if (new_dir == 4) next_pos.x -= 1;

          auto proposed_tiles = get_ship_tiles(next_pos, ship_length, new_dir, new_dir);
          bool is_valid = true;

          for (const auto& tile : proposed_tiles) {
            if (tile.x < 1 || tile.x >= max_x || tile.y < 1 || tile.y >= (max_y / 2)) {
              is_valid = false;
              break;
            }

            if (is_tile_occupied(ship_id, tile.x, tile.y)) {
              is_valid = false;
              break;
            }
          }

          if (!is_valid) continue;

          int terrain_weight = 1;
          int rotation_penalty = (new_dir == current.facing_dir) ? 0 : 1; 
          int tentative_g = g_costs[current.pos.x][current.pos.y][current.facing_dir] + terrain_weight + rotation_penalty;

          if (tentative_g < g_costs[next_pos.x][next_pos.y][new_dir]) {
            g_costs[next_pos.x][next_pos.y][new_dir] = tentative_g;

            int h_cost = std::abs(target.x - next_pos.x) + std::abs(target.y - next_pos.y);
            int f_cost = tentative_g + h_cost;
            parents[next_pos.x][next_pos.y][new_dir] = {current.pos, current.facing_dir};

            open_list.push({next_pos, new_dir, new_dir, f_cost});
          }
        }
      }

      if (found_path) {
        GridPos trace_pos = target;
        int trace_dir = end_dir;

        while (trace_pos.x != start.x || trace_pos.y != start.y) {
          final_path.push_front(trace_pos);

          ParentTracker p = parents[trace_pos.x][trace_pos.y][trace_dir];
          trace_pos = p.pos;
          trace_dir = p.dir;
        }
      }

      return final_path;
    }

    void fleet_movement_system(kitty_ecs::Registry& registry, int ship_id, float tile_size, int direction) {
      if (movement[ship_id].can_move && movement[ship_id].set_move) {
        switch(direction) {
          case 4:
            if (is_aligned(registry.transforms[ship_id].rotation, kitty_ecs::PI / 2.0f) || 
                  is_aligned(registry.transforms[ship_id].rotation, (3.0f * kitty_ecs::PI) / 2.0f)) {
              movement[ship_id].current_grid_pos.x += 1;
              movement[ship_id].facing_direction = 4;
            } else if (quadrant_check(registry.transforms[ship_id].rotation, 4) == 1) {
              registry.rotation2D(ship_id, -(kitty_ecs::PI / 4.0f));
              movement[ship_id].rot_direction = 4;
            } else if (quadrant_check(registry.transforms[ship_id].rotation, 4) == 0) {
              registry.rotation2D(ship_id, (kitty_ecs::PI / 4.0f));
              movement[ship_id].rot_direction = 4;
            }
            break;
          case 3:
            if (is_aligned(registry.transforms[ship_id].rotation, 0.0f) || 
                  is_aligned(registry.transforms[ship_id].rotation, kitty_ecs::PI) ||
                  is_aligned(registry.transforms[ship_id].rotation, (2.0f * kitty_ecs::PI))) {
              movement[ship_id].current_grid_pos.y += 1;
              movement[ship_id].facing_direction = 3;
            } else if (quadrant_check(registry.transforms[ship_id].rotation, 3) == 1) {
              registry.rotation2D(ship_id, -(kitty_ecs::PI / 4.0f));
              movement[ship_id].rot_direction = 3;
            } else if (quadrant_check(registry.transforms[ship_id].rotation, 3) == 0) {
              registry.rotation2D(ship_id, (kitty_ecs::PI / 4.0f));
              movement[ship_id].rot_direction = 3;
            }
            break;
          case 2:
            if (is_aligned(registry.transforms[ship_id].rotation, kitty_ecs::PI / 2.0f) || 
                  is_aligned(registry.transforms[ship_id].rotation, (3.0f * kitty_ecs::PI) / 2.0f)) {
              movement[ship_id].current_grid_pos.x -= 1;
              movement[ship_id].facing_direction = 2;
            } else if (quadrant_check(registry.transforms[ship_id].rotation, 2) == 1) {
              registry.rotation2D(ship_id, -(kitty_ecs::PI / 4.0f));
              movement[ship_id].rot_direction = 2;
            } else if (quadrant_check(registry.transforms[ship_id].rotation, 2) == 0) {
              registry.rotation2D(ship_id, (kitty_ecs::PI / 4.0f));
              movement[ship_id].rot_direction = 2;
            }
            break;
          case 1:
            if (is_aligned(registry.transforms[ship_id].rotation, 0.0f) || 
                  is_aligned(registry.transforms[ship_id].rotation, kitty_ecs::PI) ||
                  is_aligned(registry.transforms[ship_id].rotation, (2.0f * kitty_ecs::PI))) {
              movement[ship_id].current_grid_pos.y -= 1;
              movement[ship_id].facing_direction = 1;
            } else if (quadrant_check(registry.transforms[ship_id].rotation, 1) == 1) {
              registry.rotation2D(ship_id, -(kitty_ecs::PI / 4.0f));
              movement[ship_id].rot_direction = 1;
            } else if (quadrant_check(registry.transforms[ship_id].rotation, 1) == 0) {
              registry.rotation2D(ship_id, (kitty_ecs::PI / 4.0f));
              movement[ship_id].rot_direction = 1;
            }
            break;
        }
      }
    }

    void update_visuals(kitty_ecs::Registry& registry) {
      // 1. Reset all enemy tiles to their calm state
      for (size_t t = 0; t < tiles.size(); t++) {
        if (tiles[t].is_tile) {
          tiles[t].active = false;
          registry.colors[t] = {1.0f, 1.0f, 1.0f}; // Default white
          registry.textures[t] = {0.5f, 0.25f};    // Default enemy zone texture
        }
      }

      // 2. Loop through all living enemy ships
      for (size_t i = 0; i < stats.size(); i++) {
        if (stats[i].length == 0 || !registry.active_entities[i]) continue;

        // Fetch their current physical footprint (includes mid-rotation diagonal tiles!)
        auto footprint = get_ship_tiles(movement[i].current_grid_pos, 
            stats[i].length, movement[i].facing_direction, movement[i].rot_direction);

        // 3. Paint the footprint onto the grid
        for (const auto& pos : footprint) {
          for (size_t t = 0; t < tiles.size(); t++) {
            if (tiles[t].is_tile && tiles[t].tile_pos.x == pos.x && tiles[t].tile_pos.y == pos.y) {
              
              tiles[t].active = true;
              registry.colors[t] = {1.0f, 0.0f, 0.0f}; // Tint the tile Red
              
              // Apply your attack_marker texture (Slot 8 -> 0.0f, 0.5f)
              registry.textures[t] = {0.0f, 0.5f}; 
              break; 
            }
          }
        }
      }
    }
    
    void process_fleet_movement(kitty_ecs::Registry& registry, float tile_size, int board_x, int board_y) {
      for (size_t i = 0; i < stats.size(); i++) {
        if (stats[i].length == 0 || !registry.active_entities[i]) continue;

        auto& move_state = movement[i];
        
        if (move_state.can_move && move_state.set_move && !move_state.path_queue.empty()) {
          GridPos next_step = move_state.path_queue.front();
          if (is_tile_occupied(i, next_step.x, next_step.y)) {

            std::cout << "Ship " << i << " encountered traffic! Recalculating route...\n";

            auto new_route = calculate_path(
                move_state.current_grid_pos, 
                move_state.target_grid_pos, 
                stats[i].length, 
                move_state.facing_direction, 
                board_x, board_y, i
                );

            if (!new_route.empty()) {
              move_state.path_queue = new_route;
            } else {
              std::cout << "Ship " << i << " is completely trapped. Halting engines.\n";
              move_state.path_queue.clear();
              move_state.set_move = false;
            }

            continue; 
          }
          int dx = next_step.x - move_state.current_grid_pos.x;
          int dy = next_step.y - move_state.current_grid_pos.y;
          int curr_x = move_state.current_grid_pos.x;
          int curr_y = move_state.current_grid_pos.y;
          int face_dir = move_state.facing_direction;

          if (dy > 0 && dx == 0) {
            if (face_dir != 3 && face_dir != 1) move_state.is_rotating = true;
            else move_state.is_rotating = false;
            fleet_movement_system(registry, i, tile_size, 3);
          } else if (dy < 0 && dx == 0) {
            if (face_dir != 3 && face_dir != 1) move_state.is_rotating = true;
            else move_state.is_rotating = false;
            fleet_movement_system(registry, i, tile_size, 1);
          } else if (dx > 0 && dy == 0) {
            if (face_dir != 4 && face_dir != 2) move_state.is_rotating = true;
            else move_state.is_rotating = false;
            fleet_movement_system(registry, i, tile_size, 4);
          } else if (dx < 0 && dy == 0) {
            if (face_dir != 4 && face_dir != 2) move_state.is_rotating = true;
            else move_state.is_rotating = false;
            fleet_movement_system(registry, i, tile_size, 2);
          } else {
            move_state.is_rotating = false;
          }

          if (move_state.current_grid_pos.x == next_step.x && move_state.current_grid_pos.y == next_step.y) {
            move_state.path_queue.pop_front();
            move_state.is_rotating = false;
            if (move_state.path_queue.empty()) {
              move_state.set_move = false;
              move_state.can_move = true;
              move_state.ticks_til_move = stats[i].ticks_per_move;
            }
          }
        }
        if (move_state.ticks_til_move == 0) {
          move_state.can_move = true;
          move_state.ticks_til_move = stats[i].ticks_per_move;
        } else if (move_state.ticks_til_move <= stats[i].ticks_per_move && move_state.set_move) {
          move_state.can_move = false;
          move_state.ticks_til_move--;
        }
      }
    }
  };
}
