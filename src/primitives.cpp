#include "primitives.hpp"
#include "constants.h"
#include <cmath> // For std::cos and std::sin

namespace kitty_ecs {
  MeshData generate_quad(float size) {
    MeshData mesh;
    float half = size / 2.0f;

    mesh.vertices.push_back({{-half,  half}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}); // Top Left
    mesh.vertices.push_back({{-half, -half}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}); // Bottom Left
    mesh.vertices.push_back({{ half, -half}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}); // Bottom Right
    mesh.vertices.push_back({{ half,  half}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}); // Top Right

    mesh.indices = {0, 1, 2, 0, 2, 3};

    return mesh;
  }

  MeshData generate_circle(float radius, int segments) {
    MeshData mesh;

    mesh.vertices.push_back({{0.0f, 0.0f}});

    for (int i = 0; i < segments; ++i) {
      float angle = 2.0f * PI * float(i) / float(segments);
      float x = radius * std::cos(angle);
      float y = radius * std::sin(angle);
      mesh.vertices.push_back({{x, y}});
    }

    for (int i = 1; i <= segments; ++i) {
      mesh.indices.push_back(0); // Center point
      mesh.indices.push_back(i); // Current point

      int next = (i == segments) ? 1 : i + 1; 
      mesh.indices.push_back(next);
    }

    return mesh;
  }
}
