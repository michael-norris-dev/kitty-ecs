#pragma once
#include <vector>
#include <cstdint>
#include "constants.h"

namespace kitty_ecs {
  struct Transform {
    float x;
    float y;
    float scale_x;
    float scale_y;
    float rotation;
    int z_index;
  };

  struct Velocity {
    float dx;
    float dy;
  };

  struct Texture {
    float atlas_x;
    float atlas_y;
  };

  struct Color {
    float r, g, b;
  };

  class Registry {
    public:
      std::vector<bool> active_entities;
      std::vector<Transform> transforms;
      std::vector<Velocity> velocities;
      std::vector<Texture> textures;
      std::vector<Color> colors;

      size_t create_entity() {
        active_entities.push_back(true);
        transforms.push_back({0.0f, 0.0f, 1.0f, 1.0f, 0.0f});
        velocities.push_back({0.0f, 0.0f});
        textures.push_back({0.0f, 0.0f});
        colors.push_back({1.0f, 1.0f, 1.0f});

        return active_entities.size() - 1;
      }

      void destroy_entity(size_t entity_id) {
        if (entity_id < active_entities.size())
          active_entities[entity_id] = false;
      }

      void rotation2D(size_t entity_id, float radians) {
        transforms[entity_id].rotation += radians;
        if (transforms[entity_id].rotation > (2.0f * kitty_ecs::PI))
          transforms[entity_id].rotation -= (2.0f * kitty_ecs::PI);
        else if (transforms[entity_id].rotation < 0.0f)
          transforms[entity_id].rotation += (2.0f * kitty_ecs::PI);
      }
  };

  bool is_overlapping(const Transform& a, const Transform& b);
  void physics_system(Registry& registry);
}
