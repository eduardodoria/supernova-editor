#include "Backend.h"

#include <GLFW/glfw3.h>

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "nfd.hpp"
#include "nfd_glfw3.h"

static GLFWwindow* window = nullptr;
static nfdwindowhandle_t nativeWindow;

using namespace Supernova;

Editor::App Editor::Backend::app;
std::string Editor::Backend::title;

int Editor::Backend::init(int argc, char* argv[]) {
    // Initialize GLFW
    if (!glfwInit())
        return -1;

    if (NFD_Init() != NFD_OKAY) {
        printf("Error: NFD_Init failed: %s\n", NFD_GetError());
        return -1;
    }

    CameraRender render;

    int sampleCount = 1;

    glfwWindowHint(GLFW_SAMPLES, (sampleCount == 1) ? 0 : sampleCount);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Initialize settings first (this doesn't need ImGui yet)
    app.initializeSettings();

    // Get saved window dimensions from app
    int windowWidth = app.getInitialWindowWidth();
    int windowHeight = app.getInitialWindowHeight();

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(windowWidth, windowHeight, "Supernova Engine", NULL, NULL);
    if (!window) {
        NFD_Quit();
        glfwTerminate();
        return -1;
    }

    // Apply saved window state
    if (app.getInitialWindowMaximized()) {
        glfwMaximizeWindow(window);
    }

    NFD_GetNativeWindowFromGLFWWindow(window, &nativeWindow);

    // Make the window's context current
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context - MUST BE DONE BEFORE app.setup()
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Setup Platform/Renderer bindings - MUST BE DONE AFTER ImGui::CreateContext()
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");

    // Now we can safely call app.setup() which uses ImGui
    app.setup();
    app.engineInit(argc, argv);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    app.engineViewLoaded();

    glfwSetDropCallback(window, [](GLFWwindow* window, int count, const char** paths) {
        std::vector<std::string> droppedPaths;
        for (int i = 0; i < count; i++) {
            droppedPaths.push_back(paths[i]);
        }
        app.handleExternalDrop(droppedPaths);
    });

    // Set window close callback to handle confirmation dialogs
    glfwSetWindowCloseCallback(window, [](GLFWwindow* w) {
        // Don't close window immediately - we'll handle this ourselves
        glfwSetWindowShouldClose(w, GLFW_FALSE);

        // Call the app's exit function which will handle unsaved changes
        app.exit();
    });

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Poll and handle events
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

    // Save window size and state before closing
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    bool maximized = glfwGetWindowAttrib(window, GLFW_MAXIMIZED);

    // Save settings through app
    app.saveWindowSettings(width, height, maximized);

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    app.engineViewDestroyed();

    glfwDestroyWindow(window);
    NFD_Quit();
    glfwTerminate();

    app.engineShutdown();

    return 0;
}

Editor::App& Editor::Backend::getApp() {
    return app;
}

void Editor::Backend::disableMouseCursor() {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Editor::Backend::enableMouseCursor() {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Editor::Backend::closeWindow() {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void Editor::Backend::updateWindowTitle(const std::string& projectName) {
    if (projectName.empty()) {
        title = "Empty project - Supernova Engine";
    } else {
        title = projectName + " - Supernova Engine";
    }
    if (window){
        glfwSetWindowTitle(window, title.c_str());
    }
}

void* Editor::Backend::getNFDWindowHandle() {
    return &nativeWindow;
}