#ifndef STRUCTURE_H
#define STRUCTURE_H

#include "Project.h"
#include "SceneWindow.h"
#include "imgui.h"
#include <string>
#include <vector>
#include <filesystem>

namespace Supernova::Editor{

    struct TreeNode {
        std::string icon;
        std::string name;
        bool isScene;
        bool separator;
        bool hasTransform;
        uint32_t id;
        size_t order;
        uint32_t parent;
        std::vector<TreeNode> children;
    };

    class Structure{
    private:

        Project* project;
        SceneWindow* sceneWindow;

        char nameBuffer[256];
        char searchBuffer[256] = "";

        std::vector<TreeNode*> selectedScenes;

        Entity openParent;

        void showNewEntityMenu(bool isScene, Entity parent);
        void showIconMenu();
        void showTreeNode(TreeNode& node);
        std::string getNodeImGuiId(TreeNode& node);
        void drawInsertionMarker(const ImVec2& p1, const ImVec2& p2);
        std::string getObjectIcon(Signature signature, Scene* scene);
        TreeNode* findNode(Editor::TreeNode* root, Entity entity);
        void handleEntityFilesDrop(const std::vector<std::string>& filePaths);

    public:
        Structure(Project* project, SceneWindow* sceneWindow);

        void show();
    };

}

#endif /* STRUCTURE_H */