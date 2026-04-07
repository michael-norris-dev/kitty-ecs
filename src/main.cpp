#include <iostream>
#include <vector>
#include <string>
#include <map>
#include "engine.hpp"
#include "ecs.hpp"
#include "ships.hpp"
#include "enemy.hpp"
#include "imgui.h"

using namespace kitty_ecs;
using namespace battleship_bebop;

void set_background(Registry& registry, int density) {
  float tile_loc = -2.0 / density;
  float tile_size = 10.0 / density;
  for (int i = 0; i < density; i++) {
    for (int j = 0; j < density; j++) {
      size_t bg_id = registry.create_entity();
      float shift_factor = 1.0 - (tile_size / 10.0);
      float x = shift_factor + (i * tile_loc);
      float y = shift_factor + (j * tile_loc);

      registry.transforms[bg_id] = {x, y, tile_size, tile_size, 0.0f, -1};
      registry.colors[bg_id] = {1.0f, 1.0f, 1.0f};
      registry.textures[bg_id] = {0.0f, 0.0f};
    }
  }
}

void generate_map(Registry& registry, 
                  FleetRegistry& p_fleet, 
                  EnemyRegistry& e_fleet, 
                  int board_x, int board_y, float& tile_size) {
  tile_size = 10.0 / board_y;
  float tile_loc = -2.0 / board_y;

  for (int i = 0; i < board_x; i++) {
    for (int j = 0; j < board_y; j++) {
      size_t tile = registry.create_entity();
      float shift_factor_x = 1.0 - ((tile_size * 0.5f) / 10.0);
      float shift_factor_y = 1.0 - (tile_size / 10.0);
      float x = shift_factor_x + (i * (tile_loc * 0.5f));
      float y = shift_factor_y + (j * tile_loc);
      if (j < (int)(board_y / 2.0)) {
        e_fleet.init_tile(tile, i, j);
        registry.colors[tile] = {1.0f, 1.0f, 1.0f};
        registry.transforms[tile] = {x, y, (tile_size * 0.5f), tile_size, 0.0f, 2};
        registry.textures[tile] = {0.5f, 0.25f};
      } else {
        p_fleet.init_tile(tile, i, j);
        registry.colors[tile] = {1.0f, 1.0f, 1.0f};
        registry.transforms[tile] = {x, y, (tile_size * 0.5f), tile_size, 0.0f, 0};
        registry.textures[tile] = {0.25f, 0.25f};
      }
    }
  }
}

/*
void player_controller_system(Registry& registry, FleetRegistry& fleet, Input& input, float tile_size) {
  if (input.is_key_held(GLFW_KEY_W))
    fleet.fleet_movement_system(registry, tile_size, 1);
  if (input.is_key_held(GLFW_KEY_S))
    fleet.fleet_movement_system(registry, tile_size, 3);
  if (input.is_key_held(GLFW_KEY_D))
    fleet.fleet_movement_system(registry, tile_size, 2);
  if (input.is_key_held(GLFW_KEY_A))
    fleet.fleet_movement_system(registry, tile_size, 4);
}
*/

class BattleshipBebop : public Application {
  private:
    int board_x;
    int board_y;
    float tile_size;
    size_t player_entity;
    FleetRegistry player_fleet;
    EnemyRegistry enemy_fleet;
    size_t hover_marker;
    size_t select_marker;
    int targeting_ship_id = -1;
    bool mouse_was_pressed = false;
    std::map<std::string, int> ship_length = {
      {"Light", 3},
      {"Medium", 3},
      {"Heavy", 5},
    };
    std::map<std::string, int> ship_speed = {
      {"Light", 1},
      {"Medium", 2},
      {"Heavy", 3},
    };

  public:
    void on_start(Registry& registry, Renderer& renderer) override {
      board_y = 20;
      board_x = (int)(board_y * 2);
      set_background(registry, 2);
      generate_map(registry, player_fleet, enemy_fleet, board_x, board_y, tile_size);
      
      hover_marker = registry.create_entity();
      registry.transforms[hover_marker] = {-999.0f, -999.0f, (tile_size * 0.5f), tile_size, 0.0f, 2};
      registry.colors[hover_marker] = {0.0f, 1.0f, 1.0f};
      registry.textures[hover_marker] = {0.75f, 0.25f};

      select_marker = registry.create_entity();
      registry.transforms[select_marker] = {-999.0f, -999.0f, (tile_size * 0.5f), tile_size, 0.0f, 2};
      registry.colors[select_marker] = {0.0f, 1.0f, 0.0f};
      registry.textures[select_marker] = {0.75f, 0.25f};

      player_fleet.spawn_ship(registry, ShipTier::Heavy, 
                              ship_length["Heavy"], ship_speed["Heavy"], tile_size, board_x, board_y);
      player_fleet.spawn_ship(registry, ShipTier::Medium, 
                              ship_length["Medium"], ship_speed["Medium"], tile_size, board_x, board_y);
      player_fleet.spawn_ship(registry, ShipTier::Light, 
                              ship_length["Light"], ship_speed["Light"], tile_size, board_x, board_y);
    }

    void on_tick_update(Registry& registry, Input& input) override {
      player_fleet.process_fleet_movement(registry, tile_size, board_x, board_y);
    }

    void on_update(Registry& registry, Input& input) override {
      bool mouse_down = input.is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT);

      if (!ImGui::GetIO().WantCaptureMouse) {
        double mx, my;
        input.get_mouse_position(mx, my);
        //std::cout << "Mouse Input: " << mx << " " << my << "\n";

        int win_w, win_h;
        input.get_window_size(win_w, win_h);
        //std::cout << "Window Dims: " << win_w << " " << win_h << "\n";

        int grid_x = (board_x - 1) - static_cast<int>((mx / (double)win_w) * board_x);
        int grid_y = static_cast<int>((my / ((double)win_h - 400.0)) * board_y);
        if (targeting_ship_id != -1 && grid_x > 0 && grid_x < board_x && grid_y > (board_y / 2) && grid_y < board_y) {
          float t_loc = -2.0f / board_y;
          float shift_x = 1.0f - ((tile_size * 0.5f) / 10.0f);
          float shift_y = 1.0f - (tile_size / 10.0f);
          registry.transforms[hover_marker].x = shift_x + (grid_x * (t_loc * 0.5f));
          registry.transforms[hover_marker].y = shift_y + (grid_y * t_loc);
        } else {
          registry.transforms[hover_marker].x = -999.0f; 
        }
        bool mouse_down = input.is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT);
        if (mouse_down && !mouse_was_pressed && targeting_ship_id != -1) {
          if (grid_x > 0 && grid_x < board_x && grid_y > (board_y / 2) && grid_y < board_y) {

            auto route = player_fleet.calculate_path(
                player_fleet.movement[targeting_ship_id].current_grid_pos,
                {grid_x, grid_y},
                player_fleet.stats[targeting_ship_id].length,
                player_fleet.movement[targeting_ship_id].facing_direction,
                board_x, board_y, targeting_ship_id
                );

            if (!route.empty()) {
              player_fleet.movement[targeting_ship_id].path_queue = route;
              player_fleet.movement[targeting_ship_id].target_grid_pos = {grid_x, grid_y};
              player_fleet.movement[targeting_ship_id].set_move = true;
              registry.transforms[select_marker].x = registry.transforms[hover_marker].x;
              registry.transforms[select_marker].y = registry.transforms[hover_marker].y;
              std::cout << "Course plotted! " << route.size() << " steps to target.\n";
            } else {
              std::cout << "Invalid coordinates or path blocked!\n";
            }
          }
          targeting_ship_id = -1; 
        }
        mouse_was_pressed = mouse_down;
      }
    }

    void on_ui(Registry& registry) override {
      ImGuiWindowFlags window_flags = 
      ImGuiWindowFlags_NoTitleBar | 
      ImGuiWindowFlags_NoResize | 
      ImGuiWindowFlags_NoMove | 
      ImGuiWindowFlags_NoCollapse;

      ImGuiViewport* viewport = ImGui::GetMainViewport();
      float hud_height = 400.0f; 

      ImVec2 hud_pos = ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - hud_height);
      ImVec2 hud_size = ImVec2(viewport->WorkSize.x, hud_height);

      ImGui::SetNextWindowPos(hud_pos);
      ImGui::SetNextWindowSize(hud_size);

      ImGui::Begin("Command Interface", nullptr, window_flags);

      //ImGui::Columns(1, "HudColumns");
      //ImGui::SetColumnWidth(0, 2048);

      ImGui::SetWindowFontScale(2.0f); 
      ImGui::Text("<<< Ship Status >>>");
      ImGui::Separator();
      for (size_t i = 0; i < player_fleet.stats.size(); ++i) {
        if (player_fleet.stats[i].length == 0) continue;

        auto& state = player_fleet.movement[i];

        ImGui::Text("Ship ID: %zu", i);
        ImGui::SameLine(250);

        if (state.can_move) {
          ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[ READY ]");
          ImGui::SameLine(450);

          std::string move_label = "Plot Course##" + std::to_string(i);
          std::string attack_label = "Designate Target##" + std::to_string(i);

          if (ImGui::Button(move_label.c_str())) {
            targeting_ship_id = i;
            std::cout << "Ship " << i << " awaiting movement coordinates...\n";
          }
          if (targeting_ship_id == (int)i) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "[ TARGETING... CLICK MAP ]");
          }
          ImGui::SameLine();
          if (ImGui::Button(attack_label.c_str())) {
            std::cout << "Ship " << i << " warming up torpedoes...\n";
          }
        } else {
          ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "[ COOLDOWN ]");
          ImGui::SameLine(450);

          float progress = (float)state.ticks_til_move / (float)player_fleet.stats[i].ticks_per_move;
          ImGui::ProgressBar(progress, ImVec2(300, 0), "Charging...");

          ImGui::Dummy(ImVec2(0.0f, 10.0f));
        }
      }

      //ImGui::Columns(1);
      ImGui::End();
    }
};

int main() {
  BattleshipBebop game;
  run_engine(2048, 1024, "Test Game", &game);
  return 0;
}
