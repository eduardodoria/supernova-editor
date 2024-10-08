#ifndef LAYOUTOBJECTS_H
#define LAYOUTOBJECTS_H

#include "Project.h"
#include <string>
#include <vector>

namespace Supernova::Editor{

    struct TreeNode {
        std::string icon;
        std::string name;
        bool isScene;
        uint32_t id;
        std::vector<TreeNode> children;
    };

    class Objects{
    private:

        Project* project;

        TreeNode* selectedNodeRight;

        char nameBuffer[256];
        char searchBuffer[256] = "";

        void showNewEntityMenu();
        void showIconMenu();
        void showTreeNode(TreeNode& node);
        void changeNodeName(const TreeNode* node, const std::string name);
    public:
        Objects(Project* project);

        void show();
    };

}

#endif /* LAYOUTOBJECTS_H */