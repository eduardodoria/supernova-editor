#include "BackendGLFW.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "fonts/fa-solid-900_ttf.h"
//#include "fonts/roboto-v20-latin-regular_ttf.h"
#include "util/DefaultFont.h"
#include "external/IconsFontAwesome6.h"

#include "App.h"


using namespace Supernova;

GLFWwindow* Editor::Backend::window = nullptr;

int Editor::Backend::init(int argc, char **argv){
    // Initialize GLFW
    if (!glfwInit())
        return -1;

    Editor::App app;
    CameraRender render;

    int sampleCount = 1;

    glfwWindowHint(GLFW_SAMPLES, (sampleCount == 1) ? 0 : sampleCount);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(1280, 720, "Supernova Engine", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    //glfwMaximizeWindow(window);

    // Make the window's context current
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
    // Make sure you have a loader set up here, like glad or glew.

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking

    #ifdef _DEBUG
    //io.IniFilename = nullptr;  // Disable saving to ini file
    #endif

    io.Fonts->AddFontDefault();

    ImFontConfig config;
    config.MergeMode = true;
    config.GlyphMinAdvanceX = 16.0f;
    config.FontDataOwnedByAtlas = false;
    static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    io.Fonts->AddFontFromMemoryTTF(fa_solid_900_ttf, fa_solid_900_ttf_len, 16.0f, &config, icon_ranges);

    ImFontConfig config1;
    strcpy(config1.Name, "roboto-v20-latin-regular (16 px)");
    config1.FontDataOwnedByAtlas = false;
    config1.OversampleH = 2;
    config1.OversampleV = 2;
    config1.RasterizerMultiply = 1.5f;
    ImFont* font1 = io.Fonts->AddFontFromMemoryTTF(roboto_v20_latin_regular_ttf, roboto_v20_latin_regular_ttf_len, 16.0f, &config1);

    ImFontConfig config2;
    config2.MergeMode = true;
    config2.GlyphMinAdvanceX = 16.0f; // Use if you want to make the icon monospaced
    config2.FontDataOwnedByAtlas = false;
    static const ImWchar icon_ranges2[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    io.Fonts->AddFontFromMemoryTTF(fa_solid_900_ttf, fa_solid_900_ttf_len, 16.0f, &config2, icon_ranges2);

    io.FontDefault = font1;

    //io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    app.kewtStyleTheme();

    app.engineInit(argc, argv);

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    app.engineViewLoaded();


    // Main loop
    while (!glfwWindowShouldClose(window)){
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        app.engineRender();

        // Get window size
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        render.setClearColor(Vector4(0.45f, 0.55f, 0.60f, 1.00f));
        render.startRenderPass(display_w, display_h);

        app.show();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        render.endRenderPass();

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    app.engineViewDestroyed();

    glfwDestroyWindow(window);
    glfwTerminate();

    app.engineShutdown();

    return 0;
}

void Editor::Backend::disableMouseCursor(){
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Editor::Backend::enableMouseCursor(){
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}