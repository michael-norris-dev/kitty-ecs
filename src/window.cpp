#include <iostream>
#include <webgpu/webgpu.h>
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

GLFWwindow* initWindow(int w, int h) {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    return nullptr;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow* window = glfwCreateWindow(w, h, "DIY Graphics Engine", nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    return nullptr;
  }

  return window;
}

WGPUSurface getWindowsSurface(WGPUInstance instance, GLFWwindow* window) {
  WGPUSurfaceDescriptorFromWindowsHWND hwndDesc = {};
  hwndDesc.chain.sType = WGPUSType_SurfaceDescriptorFromWindowsHWND;
  hwndDesc.hinstance = GetModuleHandle(NULL);
  hwndDesc.hwnd = glfwGetWin32Window(window);

  WGPUSurfaceDescriptor surfaceDesc = {};
  surfaceDesc.nextInChain = (const WGPUChainedStruct*)&hwndDesc;
  
  return wgpuInstanceCreateSurface(instance, &surfaceDesc);
}

void getFramebufferSize(GLFWwindow* window, int* width, int* height) {
  glfwGetFramebufferSize(window, width, height);
  return;
}

void destroyWindow(GLFWwindow* window) {
  glfwDestroyWindow(window);
  glfwTerminate();
  return;
}
