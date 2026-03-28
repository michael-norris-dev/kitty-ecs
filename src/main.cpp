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

      registry.transforms[bg_id] = {x, y, tile_size, tile_size, 0.0, -1};
      registry.colors[bg_id] = {1.0f, 1.0f, 1.0f};
      registry.textures[bg_id] = {0.0f, 0.0f};
    }
  }
}

void generate_map(Registry& registry, EnemyRegistry& enemy_fleet, int board_size) {
  float tile_loc = -2.0 / board_size;
  float tile_size = 10.0 / board_size;

  for (int i = 0; i < (board_size * 2); i++) {
    for (int j = 0; j < board_size; j++) {
      size_t tile = registry.create_entity();
      float shift_factor_x = 1.0 - ((tile_size * 0.5f) / 10.0);
      float shift_factor_y = 1.0 - (tile_size / 10.0);
      float x = shift_factor_x + (i * (tile_loc / 2.0));
      float y = shift_factor_y + (j * tile_loc);
      if (j < (int)(board_size / 2.0)) {
        enemy_fleet.init_tile(tile, x, y);
        registry.colors[tile] = {1.0f, 1.0f, 1.0f};
        registry.transforms[tile] = {x, y, (tile_size * 0.5f), tile_size, 0.0, 2};
        registry.textures[tile] = {0.5f, 0.25f};
      } else {
        registry.colors[tile] = {1.0f, 1.0f, 1.0f};
        registry.transforms[tile] = {x, y, (tile_size * 0.5f), tile_size, 0.0, 0};
        registry.textures[tile] = {0.25f, 0.25f};
      }
    }
  }
}

void player_controller_system(Registry& registry, Input& input, size_t player_id) {
  registry.velocities[player_id].dx = 0.0f;
  registry.velocities[player_id].dy = 0.0f;

  float speed = 0.01f;

  if (input.is_key_held(GLFW_KEY_W)) registry.velocities[player_id].dy =  speed;
  if (input.is_key_held(GLFW_KEY_S)) registry.velocities[player_id].dy =  -speed;
  if (input.is_key_held(GLFW_KEY_D)) registry.velocities[player_id].dx =  speed;
  if (input.is_key_held(GLFW_KEY_A)) registry.velocities[player_id].dx =  -speed;
}

class TestGame : public Application {
  private:
    size_t player_entity;
    FleetRegistry player_fleet;
    EnemyRegistry enemy_fleet;
    std::map<std::string, int> ship_length = {
      {"Light", 2},
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
      set_background(registry, 2);
      generate_map(registry, enemy_fleet, 20);
      player_entity = registry.create_entity();
      registry.transforms[player_entity] = {-0.25f, -0.25f, 0.5f, 1.0f, 0.0, 1};
      registry.colors[player_entity] = {1.0f, 1.0f, 1.0f};
      registry.textures[player_entity] = {0.75f, 0.0f};

      player_fleet.init_ship(player_entity, ShipTier::Medium, ship_length["Medium"], ship_speed["Medium"]);
    }

    void on_update(Registry& registry, Input& input) override {
      player_controller_system(registry, input, player_entity);
      //fleet_movement_system(registry, fleet, input);
      physics_system(registry);
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

      // 2. Split the UI: Fleet Roster on the left, Global Actions on the right
      ImGui::Columns(2, "HudColumns");
      ImGui::SetColumnWidth(0, 700);

      ImGui::GetIO().FontGlobalScale = 2.0f; 
      ImGui::Text("<<< Ship Status >>>");
      for (size_t i = 0; i < player_fleet.stats.size(); ++i) {
        if (player_fleet.stats[i].length == 0) continue;

        auto& state = player_fleet.movement[i];

        ImGui::Text("Ship ID: %zu", i);
        ImGui::SameLine(100);

        if (state.can_move) {
          ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[ READY ]");
          ImGui::SameLine(200);

          // Generate unique button IDs for each ship using ImGui's ## trick
          std::string move_label = "Plot Course##" + std::to_string(i);
          std::string attack_label = "Designate Target##" + std::to_string(i);

          if (ImGui::Button(move_label.c_str())) {
            std::cout << "Ship " << i << " awaiting movement coordinates...\n";
            // Next step: switch engine to "Targeting Mode"
          }
          ImGui::SameLine();
          if (ImGui::Button(attack_label.c_str())) {
            std::cout << "Ship " << i << " warming up torpedoes...\n";
          }
        } else {
          ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "[ COOLDOWN ]");
          ImGui::SameLine(200);

          float progress = (float)state.ticks_til_move / (float)player_fleet.stats[i].ticks_per_move;
          ImGui::ProgressBar(progress, ImVec2(200, 0), "Charging...");
        }
      }

      ImGui::NextColumn();
      ImGui::Text("--- TACTICAL OPERATIONS ---");

      if (ImGui::Button("Deploy Recon Drone", ImVec2(-1, 30))) {
        std::cout << "Recon Drone deployed! Scanning sector...\n";
        // Logic to spawn a fast-moving, temporary drone entity
      }

      ImGui::Spacing();
      ImGui::TextWrapped("Intel: Sector heavily obscured. Deploy drones to reveal enemy movements.");

      ImGui::Columns(1);
      ImGui::End();
    }
};

int main() {
  TestGame game;
  run_engine(2048, 1024, "Test Game", &game);
  return 0;
}
