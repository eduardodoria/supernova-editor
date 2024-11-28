#include "Properties.h"

#include "imgui.h"

#include "external/IconsFontAwesome6.h"
#include "command/CommandHandle.h"
#include "command/type/ChangePropertyCmd.h"

using namespace Supernova;

Editor::Properties::Properties(Project* project){
    this->project = project;
}

void Editor::Properties::show(){
    ImGui::Begin("Properties");

    SceneProject* sceneProject = project->getSelectedScene();
    std::vector<Entity> entities = project->getSelectedEntities(sceneProject->id);

    std::vector<ComponentType> components;
    Entity entity = NULL_ENTITY;
    Scene* scene = sceneProject->scene;

    if (entities.size() > 0){
        entity = entities[0];
        components = Metadata::findComponents(scene, entity);

        if (ImGui::BeginTable("table_name", 2)){
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("Entity").x);
            ImGui::TableSetupColumn("Value");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Entity");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1);
            static char nameBuffer[128];
            strncpy(nameBuffer, scene->getEntityName(entity).c_str(), sizeof(nameBuffer) - 1);
            nameBuffer[sizeof(nameBuffer) - 1] = '\0';
            ImGui::InputText("##input_name", nameBuffer, IM_ARRAYSIZE(nameBuffer));
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                if (nameBuffer[0] != '\0' && strcmp(nameBuffer, scene->getEntityName(entity).c_str()) != 0) {
                    scene->setEntityName(entity, nameBuffer);
                }
            }

            ImGui::EndTable();
        }

        ImGui::Separator();

        if (ImGui::Button(ICON_FA_PLUS" New component", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
            // Button was clicked
            //ImGui::Text("Button clicked!");
        }
    }

    for (ComponentType& cpType : components){

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader(Metadata::getComponentName(cpType).c_str())){

            std::vector<PropertyData> props = Metadata::findProperties(scene, entity, cpType);

            float firstColSize = ImGui::GetFontSize();
            for (PropertyData& prop : props){
                firstColSize = std::max(firstColSize, ImGui::CalcTextSize(prop.label.c_str()).x);
            }

            ImGui::PushItemWidth(-1);
            if (ImGui::BeginTable(("table_"+Metadata::getComponentName(cpType)).c_str(), 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV)){
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, firstColSize);
                ImGui::TableSetupColumn("Value");
                for (PropertyData& prop : props){
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", prop.label.c_str());
                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(-1);

                    if (prop.type == PropertyType::Float3){
                        Vector3* value = Metadata::getPropertyRef<Vector3>(scene, entity, cpType, prop.name);
                        Vector3 newValue = *value;
                        ImGui::InputFloat3(("##input_"+prop.name).c_str(), &(newValue.x), "%.2f");
                        ImGui::SetItemTooltip("%s (X, Y, Z)", prop.label.c_str());
                        if (ImGui::IsItemDeactivatedAfterEdit()) {
                            CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new ChangePropertyCmd<Vector3>(scene, entity, cpType, prop.name, prop.updateFlags, newValue));
                        }

                    }else if (prop.type == PropertyType::Quat){
                        Quaternion* value = Metadata::getPropertyRef<Quaternion>(scene, entity, cpType, prop.name);
                        Vector3 newValueFmt = value->getEulerAngles();
                        ImGui::InputFloat3(("##input_"+prop.name).c_str(), &(newValueFmt.x), "%.2f");
                        ImGui::SetItemTooltip("%s in degrees (X, Y, Z)", prop.label.c_str());
                        if (ImGui::IsItemDeactivatedAfterEdit()) {
                            Quaternion newValue(newValueFmt.x, newValueFmt.y, newValueFmt.z);
                            CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new ChangePropertyCmd<Quaternion>(scene, entity, cpType, prop.name, prop.updateFlags, newValue));
                        }

                    }else if (prop.type == PropertyType::Bool){
                        bool* value = Metadata::getPropertyRef<bool>(scene, entity, cpType, prop.name);
                        bool newValue = *value;
                        ImGui::Checkbox(("##checkbox_"+prop.name).c_str(), &newValue);
                        ImGui::SetItemTooltip("%s", prop.label.c_str());
                        if (ImGui::IsItemDeactivatedAfterEdit()) {
                            CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new ChangePropertyCmd<bool>(scene, entity, cpType, prop.name, prop.updateFlags, newValue));
                        }
                    }



                }
                ImGui::EndTable();

                //ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y);
                //if (ImGui::TreeNodeEx("More", ImGuiTreeNodeFlags_SpanAvailWidth)){
                //    ImGui::TreePop();
                //}
            }

/*
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
*/
        }
    }
    ImGui::End();
}