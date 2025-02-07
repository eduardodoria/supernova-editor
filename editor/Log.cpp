#include "Log.h"
#include <cstdarg>
#include <iostream>

using namespace Supernova::Editor;

Output* Log::output = nullptr;

void Log::setOutputWindow(Output* output) {
    Log::output = output;
}

Output* Log::getOutputWindow() {
    return output;
}

void Log::info(const std::string& message) {
    if (output) {
        output->addLog(LogType::Info, message);
    } else {
        std::cout << "[INFO] " << message << std::endl;
    }
}

void Log::success(const std::string& message) {
    if (output) {
        output->addLog(LogType::Success, message);
    } else {
        std::cout << "[SUCCESS] " << message << std::endl;
    }
}

void Log::warning(const std::string& message) {
    if (output) {
        output->addLog(LogType::Warning, message);
    } else {
        std::cout << "[WARNING] " << message << std::endl;
    }
}

void Log::error(const std::string& message) {
    if (output) {
        output->addLog(LogType::Error, message);
    } else {
        std::cerr << "[ERROR] " << message << std::endl;
    }
}

void Log::build(const std::string& message) {
    if (output) {
        output->addLog(LogType::Build, message);
    } else {
        std::cerr << "[BUILD] " << message << std::endl;
    }
}

void Log::editor_assert(bool condition, const std::string& message) {
    if (!condition) {
        error("Assertion failed: " + message);
        #ifdef _DEBUG
            // Break into debugger if in debug mode
            #if defined(_MSC_VER)
                __debugbreak();
            #elif defined(__GNUC__)
                __builtin_trap();
            #endif
        #endif
    }
}