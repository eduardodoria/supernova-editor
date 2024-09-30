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
        std::vector<TreeNode> children;
    };

    class Objects{
    private:

        void showIconMenu();
        void showTreeNode(TreeNode& node);
    public:
        Objects();

        void show(Project* project);
    };

}

#endif /* LAYOUTOBJECTS_H */