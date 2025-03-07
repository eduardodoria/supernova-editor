#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include "imgui.h"
#include "external/IconsFontAwesome6.h"

namespace fs = std::filesystem;

namespace Supernova::Editor{
    class Widgets{

    private:
        inline static std::string shortenPath(const fs::path& path, float maxWidth) {
            std::string fullPath = path.string();
        
            ImVec2 fullPathSize = ImGui::CalcTextSize(fullPath.c_str());
            if (fullPathSize.x <= maxWidth) {
                return fullPath;
            }
        
            std::string filename = path.filename().string();
            std::string parentPath = path.parent_path().string();
        
            ImVec2 filenameSize = ImGui::CalcTextSize(filename.c_str());
            if (filenameSize.x > maxWidth) {
                std::string truncatedFilename = filename;
                while (!truncatedFilename.empty() && ImGui::CalcTextSize((truncatedFilename + "...").c_str()).x > maxWidth) {
                    truncatedFilename.pop_back();
                }
                return truncatedFilename + "...";
            }
        
            float remainingWidth = maxWidth - filenameSize.x - ImGui::CalcTextSize("/").x; // Space for '/' separator
        
            if (parentPath != ".") {
                std::string truncatedParentPath = parentPath;
                while (!truncatedParentPath.empty() && ImGui::CalcTextSize((truncatedParentPath + ".../").c_str()).x > remainingWidth) {
                    truncatedParentPath.pop_back();
                }
        
                return truncatedParentPath + ".../" + filename;
            } else {
                return filename;
            }
        }

    public:
        inline static void pathDisplay(fs::path path, fs::path projectPath = fs::path()){
            ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 50, 50, 255));
            ImGui::BeginChild("PathFrame", ImVec2(- ImGui::CalcTextSize(ICON_FA_GEAR).x - ImGui::GetStyle().ItemSpacing.x - ImGui::GetStyle().FramePadding.x * 2, ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            
            std::string subPath = path.string();
            if (subPath.find(projectPath.string()) == 0) {
                subPath = subPath.substr(projectPath.string().length());
                if (subPath.empty()){
                    subPath = "/";
                }
            }

            std::string shortenedPath = shortenPath(subPath, ImGui::GetContentRegionAvail().x);
        
            ImGui::SetCursorPosY(ImGui::GetStyle().FramePadding.y);
            ImGui::Text("%s", ((shortenedPath == ".") ? "" : shortenedPath).c_str());
        
            ImGui::EndChild();
            ImGui::SetItemTooltip("%s", path.string().c_str());
            ImGui::PopStyleColor();
        }
    };

}