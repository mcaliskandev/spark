#pragma once

#include <GLFW/glfw3.h>

#include "imgui.h"

class MainWindow {
   public:
    MainWindow();
    ~MainWindow();

    void ProcessLoop();

   private:
    const int default_window_width = 1280;
    const int default_window_height = 720;
    const char* window_title = "Spark - System Monitor";

    GLFWwindow* window;
    ImGuiIO* io;
    ImVec4 default_bg_color = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
};
