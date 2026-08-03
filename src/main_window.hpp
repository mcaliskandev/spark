#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <GLFW/glfw3.h>

#include <string>

#include "cpu_stats.hpp"
#include "imgui.h"
#include "ram_stats.hpp"
#include "version.hpp"

class MainWindow {
   public:
    MainWindow();
    ~MainWindow();

    void ProcessLoop();

   private:
    const int default_window_width = 1280;
    const int default_window_height = 720;
    const std::string window_title = "Spark - System Monitor v" SPARK_VERSION_STRING;

    GLFWwindow* window;
    ImGuiIO* io;
    ImVec4 default_bg_color = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);

    CpuStats cpu_stats;
    RamStats ram_stats;
};

#endif  // MAIN_WINDOW_HPP
