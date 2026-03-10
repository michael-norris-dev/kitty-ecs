#pragma once
#include <GLFW/glfw3.h>

class Input {
private:
  GLFWwindow* window;

public:
  Input(GLFWwindow* win) : window(win) {}

  bool is_key_held(int keycode) {
    return glfwGetKey(window, keycode) == GLFW_PRESS;
  }

  bool is_mouse_button_down(int button) {
    return glfwGetMouseButton(window, button) == GLFW_PRESS;
  }

  void get_mouse_position(double& x, double& y) {
    glfwGetCursorPos(window, &x, &y); 
  }
};
