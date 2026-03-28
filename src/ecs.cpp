#include "ecs.hpp"
#include "constants.h"

namespace kitty_ecs {
  bool is_overlapping(const Transform& a, const Transform& b) {
    float a_left   = a.x - (a.scale_x / 2.0f);
    float a_right  = a.x + (a.scale_x / 2.0f);
    float a_bottom = a.y - (a.scale_y / 2.0f);
    float a_top    = a.y + (a.scale_y / 2.0f);

    float b_left   = b.x - (b.scale_x / 2.0f);
    float b_right  = b.x + (b.scale_x / 2.0f);
    float b_bottom = b.y - (b.scale_y / 2.0f);
    float b_top    = b.y + (b.scale_y / 2.0f);

    return (a_left < b_right && 
        a_right > b_left && 
        a_bottom < b_top && 
        a_top > b_bottom);
  }

  void rotation2D(Registry& registry, size_t entity_id, double radians) {
    registry.transforms[entity_id].rotation += radians;
    if (registry.transforms[entity_id].rotation > (2.0 * PI))
      registry.transforms[entity_id].rotation -= (2.0 * PI);
  }

  void physics_system(Registry& registry) {
    for (size_t i = 0; i < registry.active_entities.size(); i++) {
      if (!registry.active_entities[i])
        continue;

      if (registry.transforms[i].x + registry.velocities[i].dx < 1.0 && registry.transforms[i].x + registry.velocities[i].dx > -1.0)
        registry.transforms[i].x += registry.velocities[i].dx;

      if (registry.transforms[i].y + registry.velocities[i].dy < 1.0 && registry.transforms[i].y + registry.velocities[i].dy > -1.0)
        registry.transforms[i].y += registry.velocities[i].dy;
    }
  }
}
