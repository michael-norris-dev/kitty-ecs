#pragma once
#include <vector>
#include <cstdint>

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

  MeshData generate_quad(float size);
  MeshData generate_circle(float radius, int segments);
}
