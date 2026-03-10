#pragma once

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIND32
#include <GLFW/glfw3native.h>

GLFWwindow* initWindow(int w, int h);
WGPUSurface getWindowsSurface(WGPUInstance instance, GLFWwindow* window);
void getFramebufferSize(GLFWwindow* window, int* width, int* height);
void destroyWindow(GLFWwindow* window);
