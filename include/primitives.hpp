#pragma once
#include <vector>
#include <cstdint>
#include <cmath>
#include "constants.h"

namespace kitty_ecs {
  struct Vertex {
    float position[2];
    float color[3];
    float uv[2];
  };

  struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
  };

  inline MeshData generate_hexagon(float radius) {
    MeshData mesh;
    // Center vertex
    mesh.vertices.push_back({{0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.5f, 0.5f}});
    float hex_width = std::sqrt(3.0f) * radius;
    float hex_height = 2.0f * radius;

    // 6 outer corners for a "Pointy-Topped" Hexagon
    for(int i = 0; i < 6; i++) {
      float angle_deg = 60.0f * i - 30.0f;
      float angle_rad = angle_deg * (kitty_ecs::PI / 180.0f);
      float x = radius * std::cos(angle_rad);
      float y = radius * std::sin(angle_rad);
      // UV mapping
      float u = (x / hex_width) + 0.5f;
      float v = (y / hex_height) + 0.5f;
      mesh.vertices.push_back({{x, y}, {1.0f, 1.0f, 1.0f}, {u, v}});
    }

    // Connect the triangles (Indices)
    for(int i = 1; i <= 6; i++) {
      mesh.indices.push_back(0);
      mesh.indices.push_back(i);
      mesh.indices.push_back((i % 6) + 1);
    }
    return mesh;
  }

  MeshData generate_quad(float size);
  MeshData generate_circle(float radius, int segments);
}
