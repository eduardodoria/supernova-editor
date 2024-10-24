#ifndef LAYOUTOBJECTS_H
#define LAYOUTOBJECTS_H

#include "Project.h"
#include "imgui.h"
#include <string>
#include <vector>

namespace Supernova::Editor{

    struct TreeNode {
        std::string icon;
        std::string name;
        bool isScene;
        bool separator;
        uint32_t id;
        std::vector<TreeNode> children;
    };

    class Objects{
    private:

        Project* project;

        TreeNode* selectedNodeRight;

        char nameBuffer[256];
        char searchBuffer[256] = "";

        void showNewEntityMenu(bool isScene);
        void showIconMenu();
        void showTreeNode(TreeNode& node);
        void changeNodeName(const TreeNode* node, const std::string name);
        void drawInsertionMarker(const ImVec2& p1, const ImVec2& p2);
        std::string getObjectIcon(Signature signature, Scene* scene);
        TreeNode* findNode(Editor::TreeNode* root, Entity entity);

    public:
        Objects(Project* project);

        void show();
    };

}

#endif /* LAYOUTOBJECTS_H */