#pragma once
#include <vector>
#include <cstdint>

// 1. Define the exact layout of our Vertex
struct Vertex {
  float position[2]; // x, y
  // You can add color[3] or uv[2] here later!
};

struct MeshData {
  std::vector<Vertex> vertices;
  std::vector<uint16_t> indices;
};

// 3. The "Menu" of available shapes
MeshData generate_quad(float size);
MeshData generate_circle(float radius, int segments);
