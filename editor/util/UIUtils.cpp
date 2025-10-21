#include "UIUtils.h"
#include "external/IconsFontAwesome6.h"

namespace Supernova::Editor {

bool UIUtils::searchInput(const char* id, std::string hint, char* buffer, size_t bufferSize, bool autoFocus) {
    ImGui::BeginGroup();
    ImGui::PushItemWidth(-1);

    if (autoFocus) {
        ImGui::SetKeyboardFocusHere();
    }

    bool changed = false;
    if (hint.empty()) {
        changed = ImGui::InputText(id, buffer, bufferSize);
    }else {
        changed = ImGui::InputTextWithHint(id, hint.c_str(), buffer, bufferSize);
    }

    // Draw the magnifying glass icon on the right with background
    ImVec2 inputMin = ImGui::GetItemRectMin();
    ImVec2 inputMax = ImGui::GetItemRectMax();

    // Get the input field background color
    ImVec4 bgColor = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);

    // Calculate icon size and padding
    ImVec2 iconSize = ImGui::CalcTextSize(ICON_FA_MAGNIFYING_GLASS);
    ImGuiStyle& style = ImGui::GetStyle();
    float framePaddingX = style.FramePadding.x;
    float framePaddingY = style.FramePadding.y;

    // Draw background rectangle for the icon
    ImVec2 iconBgMin = ImVec2(inputMax.x - iconSize.x - framePaddingX * 2, inputMin.y);
    ImVec2 iconBgMax = inputMax;
    ImGui::GetWindowDrawList()->AddRectFilled(
        iconBgMin,
        iconBgMax,
        ImGui::GetColorU32(bgColor)
    );

    // Draw the icon centered in the background
    ImVec2 iconPos = ImVec2(
        inputMax.x - iconSize.x - framePaddingX,
        inputMin.y + framePaddingY
    );
    ImGui::GetWindowDrawList()->AddText(
        iconPos,
        ImGui::GetColorU32(ImVec4(0.6f, 0.6f, 0.6f, 1.0f)),
        ICON_FA_MAGNIFYING_GLASS
    );

    ImGui::PopItemWidth();
    ImGui::EndGroup();

    return changed;
}

}