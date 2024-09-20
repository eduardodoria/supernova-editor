#include "Properties.h"

#include "imgui.h"

#include "external/IconsFontAwesome6.h"

using namespace Supernova;

Editor::Properties::Properties(){
}

void Editor::Properties::show(){
    ImGui::Begin("Properties");

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Mesh Component")){

        ImGui::SeparatorText("Common");

        ImGui::PushItemWidth(-1);
        if (ImGui::BeginTable("split2", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV)){
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFontSize() * 6);
            ImGui::TableSetupColumn("Value");
            for (int i = 0; i < 10; i++)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Texto2332");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1);
                char str0[128] = "Hello, world!";
                ImGui::InputText("##text"+i, str0, IM_ARRAYSIZE(str0));
            }
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Color");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1);
        static float col1[3] = { 1.0f, 0.0f, 0.2f };
        ImGui::ColorEdit3("color 1", col1);
            ImGui::EndTable();
        }


        ImGui::PushItemWidth(ImGui::GetFontSize() * -12);           // e.g. Leave a fixed amount of width for labels (by passing a negative value), the rest goes to widgets.

        ImGui::SeparatorText("Inputs");

        {

            // To wire InputText() with std::string or any other custom string type,
            // see the "Text Input > Resize Callback" section of this demo, and the misc/cpp/imgui_stdlib.h file.
            static char str0[128] = "Hello, world!";
            ImGui::InputText("input text", str0, IM_ARRAYSIZE(str0));

            static char str1[128] = "";
            ImGui::InputTextWithHint("input text (w/ hint)", "enter text here", str1, IM_ARRAYSIZE(str1));

            static int i0 = 123;
            ImGui::InputInt("input int", &i0);

            static float f0 = 0.001f;
            ImGui::InputFloat("input float", &f0, 0.01f, 1.0f, "%.3f");

            static double d0 = 999999.00000001;
            ImGui::InputDouble("input double", &d0, 0.01f, 1.0f, "%.8f");

            static float f1 = 1.e10f;
            ImGui::InputFloat("input scientific", &f1, 0.0f, 0.0f, "%e");

            static float vec4a[4] = { 0.10f, 0.20f, 0.30f, 0.44f };
            ImGui::InputFloat3("input float3", vec4a);
        }

        ImGui::SeparatorText("Drags");

        {
            static int i1 = 50, i2 = 42, i3 = 128;
            ImGui::DragInt("drag int", &i1, 1);

            ImGui::DragInt("drag int 0..100", &i2, 1, 0, 100, "%d%%", ImGuiSliderFlags_AlwaysClamp);
            ImGui::DragInt("drag int wrap 100..200", &i3, 1, 100, 200, "%d", ImGuiSliderFlags_WrapAround);

            static float f1 = 1.00f, f2 = 0.0067f;
            ImGui::DragFloat("drag float", &f1, 0.005f);
            ImGui::DragFloat("drag small float", &f2, 0.0001f, 0.0f, 0.0f, "%.06f ns");
            //ImGui::DragFloat("drag wrap -1..1", &f3, 0.005f, -1.0f, 1.0f, NULL, ImGuiSliderFlags_WrapAround);
        }

        ImGui::SeparatorText("Sliders");

        {
            static int i1 = 0;
            ImGui::SliderInt("slider int", &i1, -1, 3);

            static float f1 = 0.123f, f2 = 0.0f;
            ImGui::SliderFloat("slider float", &f1, 0.0f, 1.0f, "ratio = %.3f");
            ImGui::SliderFloat("slider float (log)", &f2, -10.0f, 10.0f, "%.4f", ImGuiSliderFlags_Logarithmic);
            static float angle = 0.0f;
            ImGui::SliderAngle("slider angle", &angle);

            // Using the format string to display a name instead of an integer.
            // Here we completely omit '%d' from the format string, so it'll only display a name.
            // This technique can also be used with DragInt().
            enum Element { Element_Fire, Element_Earth, Element_Air, Element_Water, Element_COUNT };
            static int elem = Element_Fire;
            const char* elems_names[Element_COUNT] = { "Fire", "Earth", "Air", "Water" };
            const char* elem_name = (elem >= 0 && elem < Element_COUNT) ? elems_names[elem] : "Unknown";
            ImGui::SliderInt("slider enum", &elem, 0, Element_COUNT - 1, elem_name); // Use ImGuiSliderFlags_NoInput flag to disable CTRL+Click here.
        }

        ImGui::SeparatorText("Selectors/Pickers");

        {
            static float col1[3] = { 1.0f, 0.0f, 0.2f };
            static float col2[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
            ImGui::ColorEdit3("color 1", col1);

            ImGui::ColorEdit4("color 2", col2);
        }

    }
    ImGui::End();
}