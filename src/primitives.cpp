#include "primitives.hpp"
#include <cmath> // For std::cos and std::sin

MeshData generate_quad(float size) {
  MeshData mesh;
  float half = size / 2.0f;

  // Push the 4 corners
  mesh.vertices.push_back({{-half,  half}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}); // Top Left
  mesh.vertices.push_back({{-half, -half}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}); // Bottom Left
  mesh.vertices.push_back({{ half, -half}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}); // Bottom Right
  mesh.vertices.push_back({{ half,  half}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}); // Top Right
  
  // Push the 6 indices (2 triangles)
  mesh.indices = {0, 1, 2, 0, 2, 3};
  
  return mesh;
}

// Example of procedural generation: A Circle
MeshData generate_circle(float radius, int segments) {
  MeshData mesh;
         
  // Center vertex is always index 0
  mesh.vertices.push_back({{0.0f, 0.0f}});

  // Generate the outer ring of vertices
  const float PI = 3.14159265359f;
  for (int i = 0; i < segments; ++i) {
    float angle = 2.0f * PI * float(i) / float(segments);
    float x = radius * std::cos(angle);
    float y = radius * std::sin(angle);
    mesh.vertices.push_back({{x, y}});
  }

  // Stitch the triangles together using indices
  for (int i = 1; i <= segments; ++i) {
    mesh.indices.push_back(0); // Center point
    mesh.indices.push_back(i); // Current point
                               
    // Next point (wrap around to 1 if we are at the end)
    int next = (i == segments) ? 1 : i + 1; 
    mesh.indices.push_back(next);
  }

  return mesh;
}
