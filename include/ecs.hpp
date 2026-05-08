#pragma once
#include <vector>
#include <cstdint>
#include "constants.h"
#include "math.hpp"

namespace kitty_ecs {
  struct Transform2D {
    Vec2 pos;
    Vec2 scale;
    float rotation; // radians
    int z_index;
  };

  struct Velocity2D {
    Vec2 velocity;
  };

  struct Transform2D {
    Vec3 position;
    Vec3 scale = {1.0f, 1.0f, 1.0f};
    Quaternion rotation;
  }

  struct RigidBody3D {
    Vec3 velocity;
    Vec3 angular_velocity;
    Vec3 force_acc;
    Vec3 torque_acc;
    float mass = 1.0f;
    float inverse_mass = 1.0f; // Pre-calculated for fast physics math
  };

  struct PIDMotor {
    float kp = 10.0f; // Proportional (Strength of the "muscle")
    float ki = 0.0f;  // Integral (Corrects long-term sag)
    float kd = 1.0f;  // Derivative (Dampens movement so it doesn't jitter)
    Quaternion target_rotation;
  };

  struct Texture {
    Vec2 atlas;
  };

  struct Color {
    Vec3 color_rgb;
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
