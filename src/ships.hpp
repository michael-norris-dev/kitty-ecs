#pragma once
#include <queue>
#include <cmath>
#include <vector>
#include <random>
#include <algorithm>
#include "ecs.hpp"
#include "math.hpp"
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

  struct MovementState {
    int ticks_til_move;
    bool can_move;
    bool set_move;
    GridPos current_grid_pos;
    std::deque<GridPos> path_queue;
    GridPos target_grid_pos;
  };

  inline bool is_in_range(GridPos entity, GridPos target, float range) {
    Vec2 d = ((float)(target.x - entity.x), (float)(target.y - entity.y));
    float distance = std::sqrt((d.x * d.x) + (d.y * d.y));
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

  struct FactionRegistry {
    std::vector<MovementState> movement;
    std::vector<PlayerTile> tiles;

    void init_entity(size_t entity_id) {
      if (entity_id >= stats.size()) {
        movement.resize(entity_id + 100);
      }
      movement[entity_id] = {0, true, false, {0, 0}};
    }

    void spawn_entity(kitty_ecs::Registry& registry, float tile_size, int board_x, int board_y) {
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
        GridPos spawn_tile = {spawn_x, spawn_y};

        if (spawn_tile.x < 1 || spawn_tile.x >= board_x - 1 || 
            spawn_tile.y < ((board_y+1) / 2) || spawn_tile.y >= (board_y-1)) {
          valid_spawn = false;
          continue;
        }
      }

      size_t entity_id = registry.create_entity();
      init_entity(entity_id);
      movement[entity_id].current_grid_pos = {spawn_x, spawn_y};

      registry.transforms[entity_id] = {-999.0f, -999.0f, 0.0f, 0.0f, 0.0f, 1};
    }

    void init_tile(size_t entity_id, int pos_x, int pos_y) {
      if (entity_id >= tiles.size()) {
        tiles.resize(entity_id + 100);
      }

      tiles[entity_id] = {{pos_x, pos_y}, false, true};
    }

    std::deque<GridPos> calculate_path(GridPos start, GridPos target, int board_x, int board_y, int entity_id) {
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
          if (next_pos.x < 0 || next_pos.x >= max_x || next_pos.y < 0 || next_pos.y >= max_y) {
            continue; 
          }

          int next_idx = next_pos.y * max_x + next_pos.x;
          int tentative_g = g_costs[current.pos.y * max_x + current.pos.x] + 1;

          if (tentative_g < g_costs[next_idx]) {
            g_costs[next_idx] = tentative_g;
            
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
    
    void movement_system(int entity_id, int dx, int dy) {
      if (movement[entity_id].can_move && movement[entity_id].set_move) {
        movement[entity_id].current_grid_pos.x += dx;
        movement[entity_id].current_grid_pos.y += dy;
        movement[entity_id].is_rotating = false;          
      }
    }

    void process_movement(kitty_ecs::Registry& registry, int board_x, int board_y) {
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
