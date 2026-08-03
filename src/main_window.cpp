#include "main_window.hpp"

#include <iostream>
#include <stdexcept>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

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
    io = &ImGui::GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
}

MainWindow::~MainWindow() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void MainWindow::ProcessLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        cpu_stats.Tick();
        ram_stats.Tick();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Spark");
        ImGui::Text("System Monitor... Loading...");
        ImGui::Text("Version %s", SPARK_VERSION_STRING);

        ImGui::Separator();
        ImGui::Text("Total CPU: %.1f%%", cpu_stats.GetTotalUsage());
        for (int i = 0; i < cpu_stats.GetCoreCount(); ++i) {
            ImGui::Text("Core %d: %.1f%%", i, cpu_stats.GetCoreUsage(i));
        }

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
