#pragma once
#include <cmath>

namespace xenoterra_imperium {
  struct SimpleNoise {
    int seed;
    SimpleNoise(int s) : seed(s) {}

    float hash(float x, float y) {
      int n = (int)x * 374761393 + (int)y * 668265263 + seed;
      n = (n ^ (n >> 13)) * 1274126177;
      return ((n & 0x0fffffff) / (float)0x0fffffff);
    }

    float smooth_noise(float x, float y) {
      float fractX = x - std::floor(x);
      float fractY = y - std::floor(y);
      float x1 = (std::floor(x));
      float y1 = (std::floor(y));
      float x2 = x1 + 1.0f;
      float y2 = y1 + 1.0f;

      float u = fractX * fractX * (3.0f - 2.0f * fractX);
      float v = fractY * fractY * (3.0f - 2.0f * fractY);

      float i1 = std::lerp(hash(x1, y1), hash(x2, y1), u);
      float i2 = std::lerp(hash(x1, y2), hash(x2, y2), u);
      return std::lerp(i1, i2, v);
    }

    // Fractal Brownian Motion (fBm) - Adds details by layering "octaves"
    float get_value(float x, float y, int octaves) {
      float total = 0.0f;
      float frequency = 1.0f;
      float amplitude = 1.0f;
      float maxValue = 0.0f;  // Used for normalizing result to 0.0 - 1.0
      for(int i=0;i<octaves;i++) {
        total += smooth_noise(x * frequency, y * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= 0.5f;
        frequency *= 2.0f;
      }
      return total / maxValue;
    }
  };
}
