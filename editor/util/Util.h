#pragma once

#include <vector>
#include <string>
#include "Backend.h"
#include "imgui.h"

namespace Supernova::Editor{
    class Util{

    public:
        inline static std::vector<std::string> getStringsFromPayload(const ImGuiPayload* payload){
            const char* data = static_cast<const char*>(payload->Data);
            size_t dataSize = payload->DataSize;

            std::vector<std::string> receivedStrings;
            size_t offset = 0;

            while (offset < dataSize) {
                // Read null-terminated strings from the payload
                const char* str = &data[offset];
                receivedStrings.push_back(std::string(str));
                offset += strlen(str) + 1; // Move past the string and null terminator
            }

            return receivedStrings;
        }
    };
}