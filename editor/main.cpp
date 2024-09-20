#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "fonts/fa-solid-900_ttf.h"
//#include "fonts/roboto-v20-latin-regular_ttf.h"
#include "external/IconsFontAwesome6.h"

#include "Supernova.h"

#include "App.h"
#include "Platform.h"

#include "GL_loader.h"
#include <GLFW/glfw3.h>

using namespace Supernova;

// Main code
int main(int argc, char** argv){

    // Initialize GLFW
    if (!glfwInit())
        return -1;

    int sampleCount = 1;

    glfwWindowHint(GLFW_SAMPLES, (sampleCount == 1) ? 0 : sampleCount);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Supernova Editor", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

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
        
    io.IniFilename = nullptr;  // Disable saving to ini file

    io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromMemoryTTF(roboto_v20_latin_regular_ttf, roboto_v20_latin_regular_ttf_len, 13.0f);

    ImFontConfig config;
    config.MergeMode = true;
    config.GlyphMinAdvanceX = 13.0f; // Use if you want to make the icon monospaced
    config.FontDataOwnedByAtlas = false;
    static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    io.Fonts->AddFontFromMemoryTTF(fa_solid_900_ttf, fa_solid_900_ttf_len, 16.0f, &config, icon_ranges);

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();




    Engine::systemInit(argc, argv);

    Scene scene;
    Polygon triangle(&scene);
    Camera camera(&scene);

    triangle.addVertex(0, -100);
    triangle.addVertex(-50, 50);
    triangle.addVertex(50, 50);

    triangle.setPosition(Vector3(300,300,0));
    triangle.setColor(0.6, 0.2, 0.6, 1);

    camera.setType(CameraType::CAMERA_2D);

    camera.setRenderToTexture(true);
    camera.setUseFramebufferSizes(false);

    scene.setCamera(&camera);

    Engine::setFixedTimeSceneUpdate(false);
    Engine::setScene(&scene);


    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");

    Editor::App app;

    Engine::systemViewLoaded();


    // Main loop
    while (!glfwWindowShouldClose(window)){
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (Editor::Platform::width != 0 && Editor::Platform::height != 0){
            camera.setFramebufferSize(Editor::Platform::width, Editor::Platform::height);
            Engine::systemDraw();
            Editor::App::texture = camera.getFramebuffer()->getRender().getColorTexture().getGLHandler();
        }

        glDisable(GL_FRAMEBUFFER_SRGB);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_SCISSOR_TEST);

        // Get window size
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        app.show();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}