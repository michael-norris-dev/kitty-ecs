#include <iostream>
#include <vector>
#include <string>
#include <map>
#include "engine.hpp"
#include "ecs.hpp"
#include "ships.hpp"
#include "imgui.h"

void set_background(Registry& registry, int density) {
  float tile_loc = -2.0 / density;
  float tile_size = 10.0 / density;
  for (int i = 0; i < density; i++) {
    for (int j = 0; j < density; j++) {
      size_t bg_id = registry.create_entity();
      float shift_factor = 1.0 - (tile_size / 10.0);
      float x = shift_factor + (i * tile_loc);
      float y = shift_factor + (j * tile_loc);

      registry.transforms[bg_id] = {x, y, tile_size, tile_size, -1};
      registry.colors[bg_id] = {1.0f, 1.0f, 1.0f};
      registry.textures[bg_id] = {0.0f, 0.0f};
    }
  }
}

void generate_map(Registry& registry) {
  int board_size = 100;
  float tile_size = -2.0 / board_size;

  for (int i = 0; i < board_size; i++) {
    for (int j = 0; j < board_size; j++) {
      size_t tile = registry.create_entity();
      float x = 1.0f + (i * tile_size);
      float y = 1.0f + (j * tile_size);

      registry.transforms[tile] = {x, y, 1.0f, 1.0f, -1};
      registry.colors[tile] = {1.0f, 1.0f, 1.0f};
      registry.textures[tile] = {0.0f, 0.0f};
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
    FleetRegistry fleet;
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
      //generate_map(registry);
      set_background(registry, 2);
      player_entity = registry.create_entity();
      registry.transforms[player_entity] = {-0.25f, -0.25f, 0.5f, 0.5f, 1};
      registry.colors[player_entity] = {1.0f, 1.0f, 1.0f};
      registry.textures[player_entity] = {0.75f, 0.0f};

      fleet.init_ship(player_entity, ShipTier::Medium, ship_length["Medium"], ship_speed["Medium"]);
    }

    void on_update(Registry& registry, Input& input) override {
      player_controller_system(registry, input, player_entity);
      //fleet_movement_system(registry, fleet, input);
      physics_system(registry);
    }

    void on_ui(Registry& registry) override {
      ImGuiWindowFlags window_flags = 
        ImGuiWindowFlags_NoTitleBar | 
        //ImGuiWindowFlags_NoBorder | 
        ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoCollapse;

      // Get the current dimensions of the main GLFW window
      ImGuiViewport* viewport = ImGui::GetMainViewport();

      // Define how tall you want the HUD to be
      float hud_height = 300.0f; 

      // Calculate the bottom-left corner of the screen
      ImVec2 hud_pos = ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - hud_height);
      ImVec2 hud_size = ImVec2(viewport->WorkSize.x, hud_height);

      // Apply the responsive positioning
      ImGui::SetNextWindowPos(hud_pos);
      ImGui::SetNextWindowSize(hud_size);

      ImGui::Begin("Command Interface", nullptr, window_flags);

      // 2. Split the UI: Fleet Roster on the left, Global Actions on the right
      ImGui::Columns(2, "HudColumns");
      ImGui::SetColumnWidth(0, 700); // Give the roster more space

      ImGui::Text("--- FLEET ROSTER ---");

      // 3. Loop through your fleet and display live statuses
      for (size_t i = 0; i < fleet.stats.size(); ++i) {
        if (fleet.stats[i].length == 0) continue; // Skip empty slots

        auto& state = fleet.movement[i];

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

          // Show a progress bar for the cooldown!
          float progress = (float)state.ticks_til_move / (float)fleet.stats[i].ticks_per_move;
          ImGui::ProgressBar(progress, ImVec2(150, 0), "Charging...");
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
  run_engine(1024, 768, "Test Game", &game);
  return 0;
}
