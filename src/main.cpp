#include <exception>
#include <iostream>

#include "boot_imgui.hpp"
#include "main_window.hpp"

int RunApp() {
    BootImgui boot_imgui;
    MainWindow main_window;
    boot_imgui.OpenGL3_Init();
    main_window.ProcessLoop();
    return 0;
}

int main() {
    try {
        RunApp();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
