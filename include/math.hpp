#pragma once
#include <cmath>

namespace kitty_ecs {
    struct Vec2 {
    float x, y;

    Vec2() : x(0.0f), y(0.0f) {}
    Vec2(float x, float y) : x(x), y(y) {}

    Vec2 operator+(const Vec2& v) const { return Vec2(x + v.x, y + v.y); }
    Vec2 operator-(const Vec2& v) const { return Vec2(x - v.x, y - v.y); }
    Vec2 operator*(float s) const { return Vec2(x * s, y * s); }
    Vec2 operator/(float s) const { return Vec2(x / s, y / s); }

    Vec2& operator+=(const Vec2& v) { x += v.x; y += v.y; return *this; }
    Vec2& operator-=(const Vec2& v) { x -= v.x; y -= v.y; return *this; }

    float magnitude() const {
      return std::sqrt(x * x + y * y);
    }

    Vec2 normalize() const {
      float mag = magnitude();
      if (mag > 0.0f) {
        return *this / mag;
      }
      return Vec2(0.0f, 0.0f);
    }

    float dot(const Vec2& v) const {
      return (x * v.x) + (y * v.y);
    }

    float signed_area(const Vec2& v) const {
      return (x * v.y) - (y * v.x);
    }
  };

  struct Vec3 {
    float x, y, z;

    Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    Vec3 operator/(float s) const { return Vec3(x / s, y / s, z / s); }

    Vec3& operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    Vec3& operator-=(const Vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }

    float magnitude() const {
      return std::sqrt(x * x + y * y + z * z);
    }

    Vec3 normalize() const {
      float mag = magnitude();
      if (mag > 0.0f) {
        return *this / mag;
      }
      return Vec3(0.0f, 0.0f, 0.0f);
    }

    float dot(const Vec3& v) const {
      return (x * v.x) + (y * v.y) + (z * v.z);
    }

    Vec3 cross(const Vec3& v) const {
      return Vec3(
          (y * v.z) - (z * v.y),
          (z * v.x) - (x * v.z),
          (x * v.y) - (y * v.x)
          );
    }
  };

  struct Quaternion {
    float w, x, y, z;

    Quaternion() : w(1.0f), x(0.0f), y(0.0f), z(0.0f) {}
    Quaternion(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}

    // Normalizing a quaternion is required after continuous physics updates
    // to prevent floating point drift.
    void normalize() {
      float mag = std::sqrt(w * w + x * x + y * y + z * z);
      if (mag > 0.0f) {
        w /= mag; x /= mag; y /= mag; z /= mag;
      }
    }

    // Quaternion multiplication combines rotations
    Quaternion operator*(const Quaternion& q) const {
      return Quaternion(
          w * q.w - x * q.x - y * q.y - z * q.z,
          w * q.x + x * q.w + y * q.z - z * q.y,
          w * q.y - x * q.z + y * q.w + z * q.x,
          w * q.z + x * q.y - y * q.x + z * q.w
          );
    }
  };

  // ------------------------------------------------------------------------
  // Mat4 (Column-Major for WebGPU)
  // ------------------------------------------------------------------------
  struct Mat4 {
    float m[16];

    Mat4() {
      for (int i = 0; i < 16; i++) m[i] = 0.0f;
      m[0] = 1.0f; m[5] = 1.0f; m[10] = 1.0f; m[15] = 1.0f;
    }

    // Perspective Projection for a First Person Camera
    // fov: Field of View in radians
    // aspect: aspect ratio (width / height)
    // near_clip, far_clip: bounds of the depth buffer
    static Mat4 perspective(float fov, float aspect, float near_clip, float far_clip) {
      Mat4 result; // Starts as identity
      result.m[0] = 1.0f; result.m[5] = 1.0f; result.m[10] = 1.0f; result.m[15] = 1.0f;

      float tan_half_fov = std::tan(fov / 2.0f);
      for (int i = 0; i < 16; i++) result.m[i] = 0.0f;

      result.m[0] = 1.0f / (aspect * tan_half_fov);
      result.m[5] = 1.0f / (tan_half_fov);
      result.m[10] = -(far_clip + near_clip) / (far_clip - near_clip);
      result.m[11] = -1.0f;
      result.m[14] = -(2.0f * far_clip * near_clip) / (far_clip - near_clip);

      return result;
    }
  };
}
