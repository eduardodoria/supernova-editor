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
        bool isScene = false;
        bool isChildScene = false;
        bool isShared = false;
        bool isParentShared = false;
        bool isMainCamera = false;
        bool separator = false;
        bool hasTransform = false;
        bool matchesSearch = false;         // Node matches search term
        bool hasMatchingDescendant = false; // Has a descendant that matches search
        uint32_t id = 0;
        size_t order = 0;
        uint32_t parent = NULL_ENTITY;
        uint32_t childSceneId = 0;          // Scene ID for child scene nodes
        uint32_t ownerSceneId = 0;          // Parent/owner scene ID (for child scene nodes)
        std::vector<TreeNode> children;
    };

    class Structure{
    private:

        Project* project;
        SceneWindow* sceneWindow;

        char nameBuffer[256];
        bool matchCase = false;
        char searchBuffer[256] = "";

        std::vector<uint32_t> selectedScenes;

        Entity openParent;

        void showNewEntityMenu(bool isScene, Entity parent, bool addToShared);
        void showIconMenu();
        void showTreeNode(TreeNode& node);
        std::string getNodeImGuiId(TreeNode& node);
        void drawInsertionMarker(const ImVec2& p1, const ImVec2& p2);
        std::string getObjectIcon(Signature signature, Scene* scene);
        TreeNode* findNode(Editor::TreeNode* root, Entity entity);
        void handleEntityFilesDrop(const std::vector<std::string>& filePaths, Entity parent = NULL_ENTITY);
        void handleSceneFilesDropAsChildScenes(const std::vector<std::string>& filePaths, uint32_t ownerSceneId);
        void showAddChildSceneMenu();

        // Search-related methods
        bool nodeMatchesSearch(const TreeNode& node, const std::string& searchLower);
        bool hasMatchingDescendant(const TreeNode& node, const std::string& searchLower);
        void markMatchingNodes(TreeNode& node, const std::string& searchLower);

    public:
        Structure(Project* project, SceneWindow* sceneWindow);

        void show();
    };

}

#endif /* STRUCTURE_H */