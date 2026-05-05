#pragma once
#include <queue>
#include <cmath>
#include <vector>
#include <random>
#include <algorithm>
#include "ecs.hpp"
#include "input.hpp"
#include "constants.h"

namespace xenoterra_imperium {
  struct GridPos {
    int x;
    int y;
  };

  struct PathNode {
    GridPos pos;
    int facing_dir;
    int rot_direction;
    int f_cost;
    bool operator>(const PathNode& other) const {
      return f_cost > other.f_cost;
    }
  };

  struct ParentTracker {
    GridPos pos;
    int dir;
  };

  struct PlayerTile {
    GridPos tile_pos;
    bool active;
    bool is_tile = false;
  };

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

    float range;
    int aoe_radius;
    int volley_count;
    float hit_chance;
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
    GridPos current_grid_pos;
    float facing_direction;
    std::deque<GridPos> path_queue;
    GridPos target_grid_pos;
    bool is_rotating;
    float rot_direction;
  };

  inline float get_orientation(int movement_dir) {
    if (movement_dir == 1 || movement_dir == 3) return 0.0f;
    return 1.0f;
  }

  CombatStats get_base_stats(ShipTier tier) {
    switch(tier) {
      case ShipTier::Heavy:  return {200, 200, 100, 50, 18.0f, 2, 6, 0.35f};
      case ShipTier::Medium: return {100, 100, 50, 25, 12.0f, 1, 3, 0.60f};
      case ShipTier::Light:  return {50, 50, 25, 0, 8.0f, 0, 1, 0.90f};
    }
    return {0, 0, 0, 0, 0.0f, 0, 0, 0.0f};
  }

  inline bool is_in_range(GridPos ship, GridPos target, float range) {
    float dx = (float)(target.x - ship.x);
    float dy = (float)(target.y - ship.y);
    float distance = std::sqrt((dx * dx) + (dy * dy));
    return distance <= range;
  }

  inline std::vector<GridPos> get_aoe_tiles(GridPos center, int aoe_radius, int board_x) {
    std::vector<GridPos> aoe;
    for (int x = center.x - aoe_radius; x <= center.x + aoe_radius; x++) {
      for (int y = center.y - aoe_radius; y <= center.y + aoe_radius; y++) {
        if (x >= 0 && x < board_x && y >= 0) {
          aoe.push_back({x, y});
        }
      }
    }
    return aoe;
  }

  inline std::vector<GridPos> get_ship_tiles(GridPos mid, int length, float rot_dir) {
    std::vector<GridPos> occupied_tiles;
    int half = (length - 1) / 2;
    occupied_tiles.push_back(mid);

    int dx = 0;
    int dy = 0;

    // Resolve the slope based on your exact fractional rules
    if (std::abs(rot_dir - 0.0f) < 0.1f)      { dx = 0; dy = 1; }  // 0.0: Vertical
    else if (std::abs(rot_dir - 1.0f) < 0.1f) { dx = 1; dy = 0; }  // 1.0: Horizontal
    else if (std::abs(rot_dir - 0.5f) < 0.1f) { dx = 1; dy = -1; } // 0.5: Diagonal (/)
    else if (std::abs(rot_dir - 1.5f) < 0.1f) { dx = 1; dy = 1; }  // 1.5: Diagonal (\)
    
    // Draw the symmetrical arms simultaneously!
    for (int i = 1; i <= half; i++) {
      occupied_tiles.push_back({mid.x + (i * dx), mid.y + (i * dy)});
      occupied_tiles.push_back({mid.x - (i * dx), mid.y - (i * dy)});
    }

    return occupied_tiles;
  }

  struct FleetRegistry {
    std::vector<ShipStats> stats;
    std::vector<MovementState> movement;
    std::vector<CombatStats> combat;
    std::vector<ActionState> actions;
    std::vector<PlayerTile> tiles;

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
      std::uniform_int_distribution<> dis_y((board_y / 2), board_y - 1);
      std::uniform_int_distribution<> dis_dir(1, 4);
      bool valid_spawn = false;
      int spawn_x, spawn_y; 
      float spawn_dir;

      while (!valid_spawn) {
        spawn_x = dis_x(gen);
        spawn_y = dis_y(gen);
        spawn_dir = get_orientation(dis_dir(gen));

        valid_spawn = true;

        auto spawn_tiles = get_ship_tiles({spawn_x, spawn_y}, ship_length, spawn_dir);

        for (const auto& tile : spawn_tiles) {
          if (tile.x < 1 || tile.x >= board_x - 1 || 
              tile.y < ((board_y+1) / 2) || tile.y >= (board_y-1)) {
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

      registry.transforms[ship_id] = {-999.0f, -999.0f, 0.0f, 0.0f, 0.0f, 1};
    }

    void init_tile(size_t entity_id, int pos_x, int pos_y) {
      if (entity_id >= tiles.size()) {
        tiles.resize(entity_id + 100);
      }

      tiles[entity_id] = {{pos_x, pos_y}, false, true};
    }

    std::deque<GridPos> calculate_path(GridPos start, GridPos target, 
        int ship_length, int start_dir, int board_x, int board_y, int ship_id) {
      std::deque<GridPos> final_path;
      int max_x = board_x;
      int max_y = board_y;

      std::vector<int> g_costs(max_x * max_y, 999999);
      std::vector<GridPos> parents(max_x * max_y, {-1, -1});

      std::priority_queue<PathNode, std::vector<PathNode>, std::greater<PathNode>> open_list;

      int start_idx = start.y * max_x + start.x;
      g_costs[start_idx] = 0;
      int start_h = std::abs(target.x - start.x) + std::abs(target.y - start.y);
      open_list.push({start, 0, 0, start_h});

      bool found_path = false;

      while (!open_list.empty()) {
        PathNode current = open_list.top();
        open_list.pop();

        if (current.pos.x == target.x && current.pos.y == target.y) {
          found_path = true;
          break; 
        }

        // --- FIX 2: True 6-Way Hexagon Pathfinding ---
        std::vector<GridPos> neighbors;
        int cx = current.pos.x;
        int cy = current.pos.y;
        
        neighbors.push_back({cx + 1, cy}); // Right
        neighbors.push_back({cx - 1, cy}); // Left
        
        if (cy % 2 == 0) { // Even Row Stagger
          neighbors.push_back({cx, cy - 1});     // Top Right
          neighbors.push_back({cx - 1, cy - 1}); // Top Left
          neighbors.push_back({cx, cy + 1});     // Bottom Right
          neighbors.push_back({cx - 1, cy + 1}); // Bottom Left
        } else {           // Odd Row Stagger
          neighbors.push_back({cx + 1, cy - 1}); // Top Right
          neighbors.push_back({cx, cy - 1});     // Top Left
          neighbors.push_back({cx + 1, cy + 1}); // Bottom Right
          neighbors.push_back({cx, cy + 1});     // Bottom Left
        }

        for (const auto& next_pos : neighbors) {
          // --- FIX 3: Fully open world (Removed the max_y / 2 boundary!) ---
          if (next_pos.x < 0 || next_pos.x >= max_x || next_pos.y < 0 || next_pos.y >= max_y) {
            continue; 
          }

          int next_idx = next_pos.y * max_x + next_pos.x;
          int tentative_g = g_costs[current.pos.y * max_x + current.pos.x] + 1;

          if (tentative_g < g_costs[next_idx]) {
            g_costs[next_idx] = tentative_g;
            
            // Hex distance heuristic
            int h_cost = std::abs(target.x - next_pos.x) + std::abs(target.y - next_pos.y); 
            int f_cost = tentative_g + h_cost;
            
            parents[next_idx] = current.pos;
            open_list.push({next_pos, 0, 0, f_cost});
          }
        }
      }

      if (found_path) {
        GridPos trace_pos = target;
        while (trace_pos.x != start.x || trace_pos.y != start.y) {
          final_path.push_front(trace_pos);
          trace_pos = parents[trace_pos.y * max_x + trace_pos.x];
        }
      }

      return final_path;
    }
    
    /*
    void update_visuals(kitty_ecs::Registry& registry) {
      for (size_t t = 0; t < tiles.size(); t++) {
        if (tiles[t].is_tile) {
          tiles[t].active = false;
          registry.colors[t] = {1.0f, 1.0f, 1.0f};
          registry.textures[t] = {0.25f, 0.25f};
        }
      }
      for (size_t i = 0; i < stats.size(); i++) {
        if (stats[i].length == 0 || !registry.active_entities[i]) continue;

        for (size_t t = 0; t < tiles.size(); t++) {
          if (tiles[t].is_tile && tiles[t].tile_pos.x == pos.x && tiles[t].tile_pos.y == pos.y) {
            tiles[t].active = true;
            registry.colors[t] = {0.0f, 1.0f, 0.0f}; // GREEN PLAYER TINT
            registry.textures[t] = {0.0f, 0.5f};     // Solid marker texture
            break;
          }
        }
      }
    }
    */
    void fleet_movement_system(int ship_id, int dx, int dy) {
      if (movement[ship_id].can_move && movement[ship_id].set_move) {
        movement[ship_id].current_grid_pos.x += dx;
        movement[ship_id].current_grid_pos.y += dy;
        movement[ship_id].is_rotating = false;          
      }
    }

    void process_fleet_movement(kitty_ecs::Registry& registry, int board_x, int board_y) {
      for (size_t i = 0; i < stats.size(); i++) {
        if (stats[i].length == 0 || !registry.active_entities[i]) continue;

        auto& move_state = movement[i];
        
        if (move_state.can_move && move_state.set_move && !move_state.path_queue.empty()) {
          GridPos next_step = move_state.path_queue.front();
          
          int dx = next_step.x - move_state.current_grid_pos.x;
          int dy = next_step.y - move_state.current_grid_pos.y;
          
          fleet_movement_system(i, dx, dy);

          if (move_state.current_grid_pos.x == next_step.x && move_state.current_grid_pos.y == next_step.y) {
            move_state.path_queue.pop_front();
            move_state.is_rotating = false;
            
            move_state.can_move = false;
            move_state.ticks_til_move = stats[i].ticks_per_move;

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
