#ifndef STRUCTURE_H
#define STRUCTURE_H

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
        size_t order;
        uint32_t parent;
        std::vector<TreeNode> children;
    };

    class Structure{
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
        Structure(Project* project);

        void show();
    };

}

#endif /* STRUCTURE_H */