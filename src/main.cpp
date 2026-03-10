#include <iostream>
#include <vector>
#include "engine.hpp"
#include "ecs.hpp"

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

public:
  void on_start(Registry& registry, Renderer& renderer) override {
    int board_size = 8;
    float square_size = 0.2f;
    float start_pos = -0.7f;

    for (int row = 0; row < board_size; row++) {
      for (int col = 0; col < board_size; col++) {
        size_t tile = registry.create_entity();
        float x = start_pos + (col * square_size);
        float y = start_pos + (row * square_size);

        registry.transforms[tile] = {x, y, 1.0f, 1.0f, 0};
        if ((row + col) % 2 == 0) {
          registry.colors[tile] = {0.9f, 0.9f, 0.8f};
        } else {
          registry.colors[tile] = {0.3f, 0.3f, 0.5f};
        }
      }
    }

    player_entity = registry.create_entity();
    registry.transforms[player_entity] = {start_pos, start_pos, 0.8f, 0.8f, 1};
    registry.colors[player_entity] = {0.1f, 0.1f, 0.1f};
  }

  void on_update(Registry& registry, Input& input) override {
    player_controller_system(registry, input, player_entity);
    physics_system(registry);
  }
};

int main() {
  TestGame game;
  run_engine(800, 600, "Test Game", &game);
  return 0;
}
