#include "boot_imgui.hpp"

#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>

#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"

BootImgui::BootImgui() {
    IMGUI_CHECKVERSION();
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW. Stopping app.\n";
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
}

void BootImgui::OpenGL3_Init() { ImGui_ImplOpenGL3_Init(glsl_version); }

void BootImgui::glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error : " << description << ". (error code : " << error
              << ")" << "\n";
}
