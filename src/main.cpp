#include <iostream>
#include <vector>
#include "engine.hpp"
#include "ecs.hpp"

void generate_map(Registry& registry) {
  int board_size = 100;
  float tile_size = -2.0 / board_size;

  for (int i = 0; i < board_size; i++) {
    for (int j = 0; j < board_size; j++) {
      size_t tile = registry.create_entity();
      float x = 1.0f + (i * tile_size);
      float y = 1.0f + (j * tile_size);
      
      registry.transforms[tile] = {x, y, 1.0f, 1.0f, 0};
      registry.colors[tile] = {0.55f, 0.85f, 0.5f};
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

public:
  void on_start(Registry& registry, Renderer& renderer) override {
    generate_map(registry);
    player_entity = registry.create_entity();
    registry.transforms[player_entity] = {-0.7f, -0.7f, 0.8f, 0.8f, 1};
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
