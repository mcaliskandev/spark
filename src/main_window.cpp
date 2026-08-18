#include "main_window.hpp"

#include <cstdio>
#include <iostream>
#include <stdexcept>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "implot.h"

namespace {

struct RingBufferHistory {
    const float* values;
    int size;
    int offset;
    double seconds_per_sample;
};

double SecondsAgo(const RingBufferHistory* history, int idx) {
    return static_cast<double>(idx - (history->size - 1)) *
           history->seconds_per_sample;
}

ImPlotPoint RingBufferGetter(int idx, void* user_data) {
    const auto* history = static_cast<const RingBufferHistory*>(user_data);
    const int i = (history->offset + idx) % history->size;
    return ImPlotPoint(SecondsAgo(history, idx),
                       static_cast<double>(history->values[i]));
}

ImPlotPoint BaselineGetter(int idx, void* user_data) {
    const auto* history = static_cast<const RingBufferHistory*>(user_data);
    return ImPlotPoint(SecondsAgo(history, idx), 0.0);
}

void DrawUsagePlot(const char* plot_id, const float* values, int size,
                   int offset, double seconds_per_sample, const ImVec4& color) {
    RingBufferHistory history{values, size, offset, seconds_per_sample};
    const double window_seconds =
        static_cast<double>(size - 1) * seconds_per_sample;

    const ImVec4 fill_color(color.x, color.y, color.z, 0.35f);
    const ImPlotSpec fill_spec(ImPlotProp_FillColor, fill_color,
                               ImPlotProp_Flags, ImPlotItemFlags_NoLegend);
    const ImPlotSpec line_spec(ImPlotProp_LineColor, color,
                               ImPlotProp_LineWeight, 2.0f, ImPlotProp_Flags,
                               ImPlotItemFlags_NoLegend);

    if (ImPlot::BeginPlot(plot_id, ImVec2(-1, 240),
                          ImPlotFlags_NoTitle | ImPlotFlags_NoMouseText |
                              ImPlotFlags_NoInputs | ImPlotFlags_NoMenus |
                              ImPlotFlags_NoBoxSelect)) {
        ImPlot::SetupAxes("Seconds ago", nullptr, ImPlotAxisFlags_Lock,
                          ImPlotAxisFlags_LockMin);
        ImPlot::SetupAxisFormat(ImAxis_X1, "%.0fs");
        ImPlot::SetupAxesLimits(-window_seconds, 0, 0, 100, ImPlotCond_Always);

        ImPlot::PlotShadedG("##fill", RingBufferGetter, &history,
                            BaselineGetter, &history, size, fill_spec);
        ImPlot::PlotLineG("##line", RingBufferGetter, &history, size,
                          line_spec);

        ImPlot::EndPlot();
    }
}

}  // namespace

MainWindow::MainWindow() {
    window = glfwCreateWindow(default_window_width, default_window_height,
                              window_title.c_str(), nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window. Stopping app.\n";
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    ImGui::CreateContext();
    ImPlot::CreateContext();
    io = &ImGui::GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);

    tray_indicator.SetQuitCallback(
        [this]() { glfwSetWindowShouldClose(window, GLFW_TRUE); });
}

MainWindow::~MainWindow() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void MainWindow::ProcessLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        cpu_stats.Tick();
        ram_stats.Tick();
        disk_stats.Tick();

        const auto now = std::chrono::steady_clock::now();
        if (now - last_tray_update >= tray_update_interval) {
            char label[64];
            std::snprintf(label, sizeof(label), "CPU : %%%.0f   RAM : %%%.0f",
                          cpu_stats.GetTotalUsage(),
                          ram_stats.GetUsagePercent());
            tray_indicator.SetLabel(label);
            last_tray_update = now;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_Once);
        ImGui::Begin("Spark");
        ImGui::Text("System Monitor... Loading...");
        ImGui::Text("Version %s", SPARK_VERSION_STRING);

        ImGui::Separator();
        ImGui::Text("Disk: %.1f%% (%.2f GB / %.2f GB)",
                    disk_stats.GetUsagePercent(), disk_stats.GetUsedGB(),
                    disk_stats.GetTotalGB());

        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_Once);
        ImGui::Begin("CPU Usage");
        DrawUsagePlot("##cpu_plot", cpu_stats.GetUsageHistory(),
                      cpu_stats.GetHistorySize(), cpu_stats.GetHistoryOffset(),
                      0.5, ImVec4(0.26f, 0.59f, 0.98f, 1.00f));

        ImGui::Separator();
        ImGui::Text("Total CPU: %.1f%%", cpu_stats.GetTotalUsage());
        for (int i = 0; i < cpu_stats.GetCoreCount(); ++i) {
            ImGui::Text("Core %d: %.1f%%", i, cpu_stats.GetCoreUsage(i));
        }
        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_Once);
        ImGui::Begin("RAM Usage");
        DrawUsagePlot("##ram_plot", ram_stats.GetUsageHistory(),
                      ram_stats.GetHistorySize(), ram_stats.GetHistoryOffset(),
                      0.5, ImVec4(0.20f, 0.80f, 0.55f, 1.00f));
        ImGui::Separator();
        ImGui::Text("RAM: %.1f%% (%.2f GB / %.2f GB)",
                    ram_stats.GetUsagePercent(), ram_stats.GetUsedGB(),
                    ram_stats.GetTotalGB());
        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(default_bg_color.x, default_bg_color.y, default_bg_color.z,
                     default_bg_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
}
