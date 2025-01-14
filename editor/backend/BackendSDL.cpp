#include "Backend.h"

#include <SDL.h>
#include <SDL_opengl.h>

#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include "nfd.hpp"
#include "nfd_sdl2.h"

static std::vector<std::string> droppedPaths;
static nfdwindowhandle_t nativeWindow;

using namespace Supernova;

// for work with mingw32
int SDL_main(int argc, char* argv[]){
    return Editor::Backend::init(argc, argv);
}

Editor::App Editor::Backend::app;

int Editor::Backend::init(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        return -1;
    }

    if (NFD_Init() != NFD_OKAY) {
        printf("Error: NFD_Init failed: %s\n", NFD_GetError());
        return -1;
    }

    CameraRender render;

    // Set GL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    int sampleCount = 1;
    if (sampleCount > 1) {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, sampleCount);
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // Create window with OpenGL context
    SDL_Window* window = SDL_CreateWindow(
        "Supernova Engine",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (!window) {
        NFD_Quit();
        SDL_Quit();
        return -1;
    }

    NFD_GetNativeWindowFromSDLWindow(window, &nativeWindow);

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        SDL_DestroyWindow(window);
        NFD_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_GL_MakeCurrent(window, glContext);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
    // Make sure you have a loader set up here, like glad or glew.

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    app.setup();
    app.engineInit(argc, argv);

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 410");

    SDL_ShowCursor(SDL_DISABLE);

    app.engineViewLoaded();

    // Main loop
    bool done = false;
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT){
                done = true;
            }
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window)){
                done = true;
            }
            if (event.type == SDL_DROPBEGIN){
                droppedPaths.clear();
            }
            if (event.type == SDL_DROPFILE){
                droppedPaths.push_back(event.drop.file);
                SDL_free(event.drop.file);
            }
            if (event.type == SDL_DROPCOMPLETE){
                app.handleExternalDrop(droppedPaths);
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        app.engineRender();

        // Get window size
        int display_w, display_h;
        SDL_GL_GetDrawableSize(window, &display_w, &display_h);

        render.setClearColor(Vector4(0.45f, 0.55f, 0.60f, 1.00f));
        render.startRenderPass(display_w, display_h);

        app.show();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        render.endRenderPass();

        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    app.engineViewDestroyed();

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    NFD_Quit();
    SDL_Quit();

    app.engineShutdown();

    return 0;
}

Editor::App& Editor::Backend::getApp() {
    return app;
}

void Editor::Backend::disableMouseCursor() {
    // TODO: cursor is not leaving the window
    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetRelativeMouseMode(SDL_TRUE);
}

void Editor::Backend::enableMouseCursor() {
    // TODO: cursor is not leaving the window
    SDL_ShowCursor(SDL_ENABLE);
    SDL_SetRelativeMouseMode(SDL_FALSE);
}

void* Editor::Backend::getNFDWindowHandle(){
    return &nativeWindow;
}