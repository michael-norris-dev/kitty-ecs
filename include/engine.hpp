#pragma once
#include "ecs.hpp"
#include "renderer.hpp"
#include "input.hpp"
#include <GLFW/glfw3.h>

namespace kitty_ecs {
  class Application {
    public:
      virtual ~Application() = default;

      virtual void on_start(Registry& registry, Renderer& renderer) = 0;
      virtual void on_tick_update(Registry& registry, Input& input) = 0;
      virtual void on_update(Registry& registry, Input& input) = 0;
      virtual void on_ui(Registry& registry) = 0;

      virtual float get_camera_x() = 0;
      virtual float get_camera_y() = 0;
      virtual float get_camera_zoom() = 0;
  };

  void run_engine(int width, int height, const char* title, Application* app);
}
