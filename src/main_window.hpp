#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <GLFW/glfw3.h>

#include <chrono>
#include <string>

#include "cpu_stats.hpp"
#include "disk_stats.hpp"
#include "imgui.h"
#include "ram_stats.hpp"
#include "tray_indicator.hpp"
#include "version.hpp"

class MainWindow {
   public:
    MainWindow();
    ~MainWindow();
    MainWindow(const MainWindow&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;

    void ProcessLoop();

   private:
    const int default_window_width = 1280;
    const int default_window_height = 720;
    const std::string window_title =
        "Spark - System Monitor v" SPARK_VERSION_STRING;

    GLFWwindow* window;
    ImGuiIO* io;
    ImVec4 default_bg_color = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);

    CpuStats cpu_stats;
    RamStats ram_stats;
    DiskStats disk_stats;

    TrayIndicator tray_indicator;
    std::chrono::steady_clock::time_point last_tray_update{};
    static constexpr std::chrono::seconds tray_update_interval{10};
};

#endif  // MAIN_WINDOW_HPP
