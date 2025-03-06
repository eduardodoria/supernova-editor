#include "Project.h"

#include "Backend.h"

#include "yaml-cpp/yaml.h"
#include <fstream>

#include "AppSettings.h"
#include "Out.h"
#include "subsystem/MeshSystem.h"
#include "command/CommandHandle.h"
#include "command/type/DeleteEntityCmd.h"
#include "Stream.h"
#include "Util.h"

using namespace Supernova;

Editor::Project::Project(){
    selectedScene = NULL_PROJECT_SCENE;

    resourcesFocused = false;

    nextSceneId = 0;
}

uint32_t Editor::Project::createNewScene(std::string sceneName){
    unsigned int nameCount = 2;
    std::string baseName = sceneName;
    bool foundName = true;
    while (foundName){
        foundName = false;
        for (auto& sceneProject : scenes) {
            std::string usedName = sceneProject.name;
            if (usedName == sceneName){
                sceneName = baseName + " " + std::to_string(nameCount);
                nameCount++;
                foundName = true;
            }
        }
    }

    SceneProject data;
    data.id = ++nextSceneId;
    data.name = sceneName;
    data.scene = new Scene();
    data.sceneRender = new SceneRender(data.scene);
    data.selectedEntities.clear();
    data.needUpdateRender = true;
    data.isModified = false;

    data.scene->setLastEntityInternal(99);

    scenes.push_back(data);

    setSelectedSceneId(scenes.back().id);

    Backend::getApp().addNewSceneToDock(scenes.back().id);

    return scenes.back().id;
}

void Editor::Project::openScene(fs::path filepath){
    try {
        YAML::Node sceneNode = YAML::LoadFile(filepath.string());

        SceneProject data;
        data.id = NULL_PROJECT_SCENE;
        data.name = "Unknown";
        data.scene = new Scene();
        data.sceneRender = new SceneRender(data.scene);
        data.selectedEntities.clear();
        data.needUpdateRender = true;
        data.isModified = false;
        data.filepath = filepath;

        Stream::decodeSceneProject(&data, sceneNode);

        if (getScene(data.id) != nullptr) {
            uint32_t old = data.id;
            data.id = ++nextSceneId;
            Out::warning("Scene with ID '%u' already exists, usind ID %u", old, data.id);
        }

        scenes.push_back(data);

        setSelectedSceneId(scenes.back().id);

        Backend::getApp().addNewSceneToDock(scenes.back().id);

    } catch (const YAML::Exception& e) {
        Log::error("Failed to open scene: %s", e.what());
        Backend::getApp().registerAlert("Error", "Failed to open scene file!");
    } catch (const std::exception& e) {
        Log::error("Failed to open scene: %s", e.what());
        Backend::getApp().registerAlert("Error", "Failed to open scene file!");
    }
}

void Editor::Project::closeScene(uint32_t sceneId) {
    if (scenes.size() == 1) {
        Log::error("Cannot close last scene");
        return;
    }

    auto it = std::find_if(scenes.begin(), scenes.end(),
        [sceneId](const SceneProject& scene) { return scene.id == sceneId; });
    
    if (it != scenes.end()) {
        if (selectedScene == sceneId) {
            Log::error("Scene is selected, cannot close it");
            return;
        }

        deleteSceneProject(&(*it));

        scenes.erase(it);
    }
}

Entity Editor::Project::createNewEntity(uint32_t sceneId, std::string entityName){
    for (int i = 0; i < scenes.size(); i++){
        if (scenes[i].id == sceneId){
            Entity entity = scenes[i].scene->createEntity();
            scenes[i].scene->setEntityName(entity, entityName);

            scenes[i].entities.push_back(entity);

            setSelectedEntity(sceneId, entity);

            return entity;
        }
    }

    return NULL_ENTITY;
}

bool Editor::Project::createNewComponent(uint32_t sceneId, Entity entity, ComponentType component){
    for (int i = 0; i < scenes.size(); i++){
        if (scenes[i].id == sceneId){
            if (component == ComponentType::Transform){
                scenes[i].scene->addComponent<Transform>(entity, {});
            }
            if (component == ComponentType::MeshComponent){
                scenes[i].scene->addComponent<MeshComponent>(entity, {});
            }
            return true;
        }
    }

    return false;
}

void Editor::Project::deleteSceneProject(SceneProject* sceneProject){
    delete sceneProject->sceneRender;
    delete sceneProject->scene;
}

void Editor::Project::createEmptyProject(std::string projectName) {
    if (hasScenesUnsavedChanges() || isTempUnsavedProject()) {
        Backend::getApp().registerConfirmAlert(
            "Unsaved Changes",
            "There are unsaved changes. Do you want to save them before creating a new project?",
            [this, projectName]() {
                // Yes callback - save all and then reset
                saveProject(true);
                saveAllScenes();
                createTempProject(projectName, true);
            },
            [this, projectName]() {
                // No callback - just reset without saving
                createTempProject(projectName, true);
            }
        );
    } else {
        // No unsaved changes, just reset
        createTempProject(projectName, true);
    }
}

void Editor::Project::resetConfigs() {
    // Clear existing scenes
    for (auto& sceneProject : scenes) {
        deleteSceneProject(&sceneProject);
    }
    scenes.clear();
    Backend::getApp().resetLastActivatedScene();

    // Reset state
    selectedScene = NULL_PROJECT_SCENE;
    nextSceneId = 0;
    projectPath.clear();

    //createNewScene("New Scene");
}

bool Editor::Project::openProjectFileDialog() {
    // Get user's home directory as default path
    std::string homeDirPath;
    #ifdef _WIN32
    homeDirPath = std::filesystem::path(getenv("USERPROFILE")).string();
    #else
    homeDirPath = std::filesystem::path(getenv("HOME")).string();
    #endif

    // Open a folder selection dialog
    std::string selectedDir = Util::openFileDialog(homeDirPath, false, true);

    if (selectedDir.empty()) {
        return false; // User canceled the dialog
    }

    std::filesystem::path projectDir = std::filesystem::path(selectedDir);
    std::filesystem::path projectFile = projectDir / "project.yaml";

    // Check if the selected directory contains a project.yaml file
    if (!std::filesystem::exists(projectFile)) {
        Backend::getApp().registerAlert("Error", "The selected directory is not a valid project. No project.yaml file found!");
        return false;
    }

    if (loadProject(projectDir)) {
        return true;
    } else {
        Out::error("Failed to open project: %s", projectDir.string().c_str());
        Backend::getApp().registerAlert("Error", "Failed to open project!");
        return false;
    }
}

bool Editor::Project::createTempProject(std::string projectName, bool deleteIfExists) {
    try {
        resetConfigs();

        // Clear the last project path in settings when creating a new temp project
        AppSettings::setLastProjectPath(std::filesystem::path());

        projectPath = std::filesystem::temp_directory_path() / projectName;
        fs::path projectFile = projectPath / "project.yaml";

        if (deleteIfExists && fs::exists(projectPath)) {
            fs::remove_all(projectPath);
        }

        if (!std::filesystem::exists(projectFile)) {
            if (!std::filesystem::exists(projectPath)) {
                std::filesystem::create_directory(projectPath);
            }
            Out::info("Created project directory: %s", projectPath.string().c_str());
            saveProject();
            createNewScene("New Scene");
        } else {
            Out::info("Project directory already exists: %s", projectPath.string().c_str());
            loadProject(projectPath);
        }

        Backend::getApp().updateResourcesPath();

    } catch (const std::exception& e) {
        printf("Error: %s\n", e.what());
        return false;
    }

    return true;
}

void Editor::Project::saveProject(bool userCalled) {
    // Check if the project is currently in a temporary directory
    if (isTempProject() && userCalled) {
        std::string homeDirPath;
        #ifdef _WIN32
        homeDirPath = std::filesystem::path(getenv("USERPROFILE")).string();
        #else
        homeDirPath = std::filesystem::path(getenv("HOME")).string();
        #endif
        // Open a dialog to choose an empty folder to save the project
        std::string saveDirPath = Util::saveFileDialog(homeDirPath, "MyProject", false);

        if (saveDirPath.empty()) {
            return; // User canceled the dialog
        }

        std::filesystem::path newProjectPath = std::filesystem::path(saveDirPath);

        AppSettings::setLastProjectPath(newProjectPath);

        // Check if the directory exists and is empty
        if (std::filesystem::exists(newProjectPath)) {
            if (!std::filesystem::is_empty(newProjectPath)) {
                Backend::getApp().registerAlert("Error", "Please select an empty directory for the project!");
                return;
            }
        } else {
            // Create the directory if it doesn't exist
            try {
                std::filesystem::create_directory(newProjectPath);
            } catch (const std::exception& e) {
                Out::error("Failed to create project directory: %s", e.what());
                Backend::getApp().registerAlert("Error", "Failed to create project directory!");
                return;
            }
        }

        // Move all project files from temp dir to the new location
        try {
            for (const auto& entry : std::filesystem::directory_iterator(projectPath)) {
                std::filesystem::path destPath = newProjectPath / entry.path().filename();
                std::filesystem::copy(entry.path(), destPath, 
                                     std::filesystem::copy_options::recursive);
            }

            // Update all scene filepaths to the new location
            for (auto& sceneProject : scenes) {
                if (!sceneProject.filepath.empty()) {
                    std::filesystem::path relativePath = std::filesystem::relative(
                        sceneProject.filepath, projectPath);
                    sceneProject.filepath = newProjectPath / relativePath;
                }
            }

            // Delete the temp directory after moving all files
            std::filesystem::remove_all(projectPath);

            // Update the project path and set tempPath to false
            projectPath = newProjectPath;

            Backend::getApp().updateResourcesPath();

            //saveAllScenes();

            Out::info("Project moved from temporary directory to: %s", projectPath.string().c_str());

        } catch (const std::exception& e) {
            Out::error("Failed to move project files: %s", e.what());
            Backend::getApp().registerAlert("Error", "Failed to move project files to the new location!");
            return;
        }
    }

    // Now save the project file
    YAML::Node root = Stream::encodeProject(this);

    std::filesystem::path projectFile = projectPath / "project.yaml";
    std::ofstream fout(projectFile.string());
    fout << YAML::Dump(root);
    fout.close();

    if (userCalled){
        Out::info("Project saved to: %s", projectPath.string().c_str());
    }
}

bool Editor::Project::loadProject(const std::filesystem::path path) {
    resetConfigs();

    projectPath = path;

    try {
        if (!std::filesystem::exists(projectPath)) {
            Out::error("Project directory does not exist: %s", projectPath.string().c_str());
            return false;
        }

        std::filesystem::path projectFile = projectPath / "project.yaml";
        if (!std::filesystem::exists(projectFile)) {
            Out::error("Project file does not exist: %s", projectFile.string().c_str());
            return false;
        }

        // Load and parse project file
        YAML::Node projectNode = YAML::LoadFile(projectFile.string());
        Stream::decodeProject(this, projectNode);

        // Create a default scene if no scenes were loaded
        if (scenes.empty()) {
            createNewScene("New Scene");
        }

        Backend::getApp().updateResourcesPath();

        // Save this as the last opened project
        if (!isTempProject()) {
            AppSettings::setLastProjectPath(projectPath);
        }

        Out::info("Project loaded successfully: %s", projectPath.string().c_str());
        return true;

    } catch (const YAML::Exception& e) {
        Out::error("Failed to load project YAML: %s", e.what());
        Backend::getApp().registerAlert("Error", "Failed to load project file!");
        return false;
    } catch (const std::exception& e) {
        Out::error("Failed to load project: %s", e.what());
        Backend::getApp().registerAlert("Error", "Failed to load project!");
        return false;
    }
}

bool Editor::Project::openProject() {
    if (hasScenesUnsavedChanges() || isTempUnsavedProject()) {
        Backend::getApp().registerConfirmAlert(
            "Unsaved Changes",
            "There are unsaved changes. Do you want to save them before opening another project?",
            [this]() {
                // Yes callback - save all and then continue
                saveProject(true);
                saveAllScenes();
                // Only after saving, proceed with file dialog
                this->openProjectFileDialog();
            },
            [this]() {
                // No callback - just continue without saving
                this->openProjectFileDialog();
            }
        );
        return true; // Operation in progress
    } else {
        // No unsaved changes, proceed directly
        return openProjectFileDialog();
    }
}

void Editor::Project::saveScene(uint32_t sceneId) {
    SceneProject* sceneProject = getScene(sceneId);
    if (!sceneProject) {
        Out::error("Cannot save scene - invalid scene ID: %u", sceneId);
        return;
    }

    // If filepath is already set, just save to that path
    if (!sceneProject->filepath.empty()) {
        saveSceneToPath(sceneId, sceneProject->filepath);
        return;
    }

    // Otherwise show save dialog through the App
    Backend::getApp().registerSaveSceneDialog(sceneId);
}

void Editor::Project::saveSceneToPath(uint32_t sceneId, const std::filesystem::path& path) {
    SceneProject* sceneProject = getScene(sceneId);
    if (!sceneProject) {
        return;
    }

    YAML::Node root = Stream::encodeSceneProject(sceneProject);
    std::ofstream fout(path.string());
    fout << YAML::Dump(root);
    fout.close();

    sceneProject->filepath = path;
    sceneProject->isModified = false;
    saveProject();

    Out::info("Scene saved to: %s", path.string().c_str());
}

void Editor::Project::saveAllScenes() {
    for (auto& sceneProject : scenes) {
        if (sceneProject.isModified) {
            saveScene(sceneProject.id);
        }
    }
}

void Editor::Project::saveLastSelectedScene(){
    saveScene(selectedScene);
}

void Editor::Project::deleteEntity(uint32_t sceneId, Entity entity){
    deleteEntities(sceneId, {entity});
}

void Editor::Project::deleteEntities(uint32_t sceneId, std::vector<Entity> entities){
    DeleteEntityCmd* deleteCmd = nullptr;

    Scene* scene = getScene(sceneId)->scene;
    auto transforms = scene->getComponentArray<Transform>();

     for(Entity& entity : entities){
        size_t firstIndex = transforms->getIndex(entity);
        size_t branchIndex = scene->findBranchLastIndex(entity);
        for (int t = branchIndex; t >= (firstIndex + 1); t--) {
            Entity childEntity = transforms->getEntity(t);
            if (childEntity != NULL_ENTITY) {
                deleteCmd = new DeleteEntityCmd(this, sceneId, childEntity);
                CommandHandle::get(getSelectedSceneId())->addCommand(deleteCmd);
            }
        }

        deleteCmd = new DeleteEntityCmd(this, sceneId, entity);
        CommandHandle::get(getSelectedSceneId())->addCommand(deleteCmd);
     }

     if (deleteCmd){
        deleteCmd->setNoMerge(); // the last
     }
}

Entity Editor::Project::findObjectByRay(uint32_t sceneId, float x, float y){
    SceneProject* scenedata = getScene(sceneId);
    Ray ray = scenedata->sceneRender->getCamera()->screenToRay(x, y);

    float distance = FLT_MAX;
    Entity selEntity = NULL_ENTITY;
    for (auto& entity : scenedata->entities) {
        MeshComponent* mesh = scenedata->scene->findComponent<MeshComponent>(entity);
        if (mesh){
            RayReturn rreturn = ray.intersects(mesh->worldAABB);
            if (rreturn.hit){
                if (rreturn.distance < distance){
                    distance = rreturn.distance;
                    selEntity = entity;
                }
            }
        }
    }

    return selEntity;
}

bool Editor::Project::selectObjectByRay(uint32_t sceneId, float x, float y, bool shiftPressed){
    SceneProject* scenedata = getScene(sceneId);
    Entity selEntity = findObjectByRay(sceneId, x, y);

    if (!scenedata->sceneRender->isGizmoSideSelected()){
        if (selEntity != NULL_ENTITY){
            if (!shiftPressed){
                clearSelectedEntities(sceneId);
            }
            addSelectedEntity(sceneId, selEntity);
            return true;
        }

        clearSelectedEntities(sceneId);
    }

    return false;
}

bool Editor::Project::selectObjectsByRect(uint32_t sceneId, Vector2 start, Vector2 end){
    SceneProject* scenedata = getScene(sceneId);
    Camera* camera = scenedata->sceneRender->getCamera();

    clearSelectedEntities(sceneId);

    float distance = FLT_MAX;
    Entity selEntity = NULL_ENTITY;
    for (auto& entity : scenedata->entities) {
        Transform* transform = scenedata->scene->findComponent<Transform>(entity);
        MeshComponent* mesh = scenedata->scene->findComponent<MeshComponent>(entity);
        if (transform && mesh){
            const Vector3* corners = mesh->aabb.getAllCorners();

            Vector2 minRect = Vector2(std::min(start.x, end.x), std::min(start.y, end.y));
            Vector2 maxRect = Vector2(std::max(start.x, end.x), std::max(start.y, end.y));

            bool found = true;

            for (int c = 0; c < 8; c++){
                Vector4 clipCorner = camera->getViewProjectionMatrix() * transform->modelMatrix * Vector4(corners[c], 1.0);
                Vector3 ndcCorner = Vector3(clipCorner) / clipCorner.w;

                if (!(ndcCorner.x >= minRect.x && ndcCorner.x <= maxRect.x && ndcCorner.y >= minRect.y && ndcCorner.y <= maxRect.y)){
                    found = false;
                    break;
                }
            }

            if (found){
                //printf("Found entity %u\n", entity);
                addSelectedEntity(sceneId, entity);
            }
        }
    }

    return false;
}

std::vector<Editor::SceneProject>&  Editor::Project::getScenes(){
    return scenes;
}

template<typename T>
T* Editor::Project::findScene(uint32_t sceneId) const {
    for (int i = 0; i < scenes.size(); i++) {
        if (scenes[i].id == sceneId) {
            return const_cast<T*>(&scenes[i]);
        }
    }
    return nullptr;
}


// Non-const version
Editor::SceneProject* Editor::Project::getScene(uint32_t sceneId) {
    return findScene<Editor::SceneProject>(sceneId);
}

// Const version
const Editor::SceneProject* Editor::Project::getScene(uint32_t sceneId) const {
    return findScene<const Editor::SceneProject>(sceneId);
}

Editor::SceneProject* Editor::Project::getSelectedScene(){
    return getScene(selectedScene);
}

const Editor::SceneProject* Editor::Project::getSelectedScene() const{
    return getScene(selectedScene);
}

void Editor::Project::setNextSceneId(uint32_t nextSceneId){
    this->nextSceneId = nextSceneId;
}

uint32_t Editor::Project::getNextSceneId() const{
    return nextSceneId;
}

void Editor::Project::setSelectedSceneId(uint32_t selectedScene){
    if (this->selectedScene != selectedScene){
        this->selectedScene = selectedScene;
    }
}

uint32_t Editor::Project::getSelectedSceneId() const{
    return selectedScene;
}

bool Editor::Project::isTempProject() const{
    std::error_code ec;
    auto relPath = std::filesystem::relative(projectPath, std::filesystem::temp_directory_path(), ec);

    if (ec) {
        return false;
    }

    return relPath.string().find("..") == std::string::npos;
}

bool Editor::Project::isTempUnsavedProject() const{
    bool isTemp = isTempProject();

    if (isTemp){
        for (auto& scene : scenes){
            if (!scene.filepath.empty() || scene.isModified){
                return true;
            }
        }
    }

    return false;
}

std::filesystem::path Editor::Project::getProjectPath() const{
    return projectPath;
}

void Editor::Project::replaceSelectedEntities(uint32_t sceneId, std::vector<Entity> selectedEntities){
    getScene(sceneId)->selectedEntities = selectedEntities;
}

void Editor::Project::setSelectedEntity(uint32_t sceneId, Entity selectedEntity){
    std::vector<Entity>& entities = getScene(sceneId)->selectedEntities;

    entities.clear();
    if (selectedEntity != NULL_ENTITY){
        entities.push_back(selectedEntity);
    }
}

void Editor::Project::addSelectedEntity(uint32_t sceneId, Entity selectedEntity){
    std::vector<Entity>& entities = getScene(sceneId)->selectedEntities;
    Scene* scene = getScene(sceneId)->scene;
    auto transforms = scene->getComponentArray<Transform>();

    if (selectedEntity != NULL_ENTITY){
        if (std::find(entities.begin(), entities.end(), selectedEntity) == entities.end()) {
            entities.push_back(selectedEntity);
        }
    }

    // removing childs of selected entities
    std::vector<Entity> removeChilds;
    for (auto& entity: entities){
        if (scene->getSignature(entity).test(scene->getComponentId<Transform>())){
            size_t firstIndex = transforms->getIndex(entity);
            size_t branchIndex = scene->findBranchLastIndex(entity);
            for (int t = (firstIndex+1); t <= branchIndex; t++){
                Entity childEntity = transforms->getEntity(t);
                if (std::find(entities.begin(), entities.end(), childEntity) != entities.end()) {
                    removeChilds.push_back(childEntity);
                    #ifdef _DEBUG
                    printf("DEBUG: Removed entity %u from selection\n", childEntity);
                    #endif
                }
            }
        }
    }
    entities.erase(
        std::remove_if(entities.begin(), entities.end(), [&removeChilds](Entity value) {
            return std::find(removeChilds.begin(), removeChilds.end(), value) != removeChilds.end();
        }),
        entities.end()
    );
}

bool Editor::Project::isSelectedEntity(uint32_t sceneId, Entity selectedEntity){
    std::vector<Entity>& entities = getScene(sceneId)->selectedEntities;

    if (std::find(entities.begin(), entities.end(), selectedEntity) != entities.end()) {
        return true;
    }

    return false;
}

void Editor::Project::clearSelectedEntities(uint32_t sceneId){
    getScene(sceneId)->selectedEntities.clear();
}

std::vector<Entity> Editor::Project::getSelectedEntities(uint32_t sceneId) const{
    return getScene(sceneId)->selectedEntities;
}

bool Editor::Project::hasSelectedEntities(uint32_t sceneId) const{
    return (getScene(sceneId)->selectedEntities.size() > 0);
}

bool Editor::Project::hasSelectedSceneUnsavedChanges() const{
    return getScene(selectedScene)->isModified;
}

bool Editor::Project::hasScenesUnsavedChanges() const{
    for (auto& scene: scenes){
        if (scene.isModified){
            return true;
        }
    }
    return false;
}

void Editor::Project::build() {
    generator.build(getProjectPath());

    std::thread connectThread([this]() {
        generator.waitForBuildToComplete();
        if (conector.connect(getProjectPath())) {
            conector.execute();
        }
    });
    connectThread.detach();
}