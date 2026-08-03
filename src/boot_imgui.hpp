#ifndef BOOT_IMGUI_HPP
#define BOOT_IMGUI_HPP

class BootImgui {
   public:
    BootImgui();

    void OpenGL3_Init();

   private:
    static void glfw_error_callback(int error, const char* description);

    const char* glsl_version = "#version 130";
};

#endif  // BOOT_IMGUI_HPP
