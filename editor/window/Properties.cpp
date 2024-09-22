#include "Properties.h"

#include "imgui.h"

#include "external/IconsFontAwesome6.h"

using namespace Supernova;

Editor::Properties::Properties(){
}

void Editor::Properties::show(){
    ImGui::Begin("Properties");

    float firstColSize = ImGui::GetFontSize() * 4;

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Mesh Component")){

        ImGui::SeparatorText("Common");

        ImGui::PushItemWidth(-1);
        if (ImGui::BeginTable("split2", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV)){
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, firstColSize);
            ImGui::TableSetupColumn("Value");
            
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Texto1");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1);
            static char str0[128] = "Hello, world!";
            ImGui::InputText("##input text", str0, IM_ARRAYSIZE(str0));

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Texto2");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1);
            static char str1[128] = "";
            ImGui::InputTextWithHint("##input text (w/ hint)", "enter text here", str1, IM_ARRAYSIZE(str1));
            
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Int");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1);
            static int i0 = 123;
            ImGui::InputInt("##input int", &i0);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Float");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1);
            static float f0 = 0.001f;
            ImGui::InputFloat("##input float", &f0, 0.01f, 1.0f, "%.3f");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Vector3");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1);
            static float vec4a[4] = { 0.10f, 0.20f, 0.30f, 0.44f };
            ImGui::InputFloat3("##input float3", vec4a);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Color");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1);
            static float col1[3] = { 1.0f, 0.0f, 0.2f };
            ImGui::ColorEdit3("color 1", col1);

            ImGui::EndTable();
        }

        ImGui::SeparatorText("Drags");

        ImGui::PushItemWidth(-1);
        if (ImGui::BeginTable("split2", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV)){
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, firstColSize);
            ImGui::TableSetupColumn("Value");
            
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Int");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1);
            static int i1 = 0;
            ImGui::SliderInt("##slider int", &i1, -1, 3);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Float");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1);
            static float f1 = 0.123f, f2 = 0.0f;
            ImGui::SliderFloat("##slider float", &f1, 0.0f, 1.0f, "ratio = %.3f");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Angle");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1);
            static float angle = 0.0f;
            ImGui::SliderAngle("##slider angle", &angle);

            ImGui::EndTable();
        }

    }
    ImGui::End();
}