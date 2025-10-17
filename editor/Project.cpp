#include "Project.h"

#include "Backend.h"

#include <fstream>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <unordered_set>

#include "render/SceneRender2D.h"
#include "render/SceneRender3D.h"

#include "AppSettings.h"
#include "Out.h"
#include "subsystem/MeshSystem.h"
#include "command/CommandHandle.h"
#include "command/type/DeleteEntityCmd.h"
#include "command/type/CreateEntityCmd.h"
#include "command/type/MoveEntityOrderCmd.h"
#include "Stream.h"
#include "util/FileDialogs.h"
#include "util/SHA1.h"
#include "util/GraphicUtils.h"
#include "util/ProjectUtils.h"

using namespace Supernova;

Editor::Project::Project(){
    resetConfigs();
}

std::string Editor::Project::getName() const {
    return name; 
}

void Editor::Project::setName(std::string name){
    this->name = name;
    Backend::updateWindowTitle(name);
}

void Editor::Project::setWindowSize(unsigned int width, unsigned int height){
    this->windowWidth = width;
    this->windowHeight = height;
}

unsigned int Editor::Project::getWindowWidth() const{
    return windowWidth;
}

unsigned int Editor::Project::getWindowHeight() const{
    return windowHeight;
}

uint32_t Editor::Project::createNewScene(std::string sceneName, SceneType type){
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
    data.sceneType = type;
    if (data.sceneType == SceneType::SCENE_3D){
        data.sceneRender = new SceneRender3D(data.scene);
    }else if (data.sceneType == SceneType::SCENE_2D){
        data.sceneRender = new SceneRender2D(data.scene, windowWidth, windowHeight, false);
    }else if (data.sceneType == SceneType::SCENE_UI){
        data.sceneRender = new SceneRender2D(data.scene, windowWidth, windowHeight, true);
    }
    data.selectedEntities.clear();
    data.needUpdateRender = true;
    data.isModified = true;
    data.isVisible = true;
    data.playState = ScenePlayState::STOPPED;

    pauseEngineScene(&data, true);

    scenes.push_back(data);

    setSelectedSceneId(data.id);

    if (data.sceneType == SceneType::SCENE_3D){
        CreateEntityCmd sunCreator(this, data.id, "Sun", EntityCreationType::DIRECTIONAL_LIGHT);
        sunCreator.addProperty<Vector3>(ComponentType::Transform, "position", Vector3(0.0f, 10.0f, 0.0f));
        sunCreator.addProperty<float>(ComponentType::LightComponent, "intensity", 4.0f);
        sunCreator.addProperty<Vector3>(ComponentType::LightComponent, "direction", Vector3(-0.2f, -0.5f, 0.3f));
        sunCreator.addProperty<bool>(ComponentType::LightComponent, "shadows", true);
        sunCreator.addProperty<float>(ComponentType::LightComponent, "range", 100);
        sunCreator.execute();

        clearSelectedEntities(data.id);
    }

    Backend::getApp().addNewSceneToDock(data.id);

    return data.id;
}

void Editor::Project::openScene(fs::path filepath){
    try {
        YAML::Node sceneNode = YAML::LoadFile(filepath.string());

        SceneProject data;
        data.id = NULL_PROJECT_SCENE;
        data.name = "Unknown";
        data.scene = nullptr;
        data.selectedEntities.clear();
        data.needUpdateRender = true;
        data.isModified = false;
        data.filepath = filepath;
        data.playState = ScenePlayState::STOPPED;

        Stream::decodeSceneProject(&data, sceneNode);

        if (data.sceneType == SceneType::SCENE_3D){
            data.sceneRender = new SceneRender3D(data.scene);
        }else if (data.sceneType == SceneType::SCENE_2D){
            data.sceneRender = new SceneRender2D(data.scene, windowWidth, windowHeight, false);
        }else if (data.sceneType == SceneType::SCENE_UI){
            data.sceneRender = new SceneRender2D(data.scene, windowWidth, windowHeight, true);
        }

        Stream::decodeSceneProjectEntities(this, &data, sceneNode);

        if (getScene(data.id) != nullptr) {
            uint32_t old = data.id;
            data.id = ++nextSceneId;
            Out::warning("Scene with ID '%u' already exists, usind ID %u", old, data.id);
        }

        pauseEngineScene(&data, true);

        scenes.push_back(data);

        setSelectedSceneId(scenes.back().id);

        Backend::getApp().addNewSceneToDock(scenes.back().id);

    } catch (const YAML::Exception& e) {
        Out::error("Failed to open scene: %s", e.what());
        Backend::getApp().registerAlert("Error", "Failed to open scene file!");
    } catch (const std::exception& e) {
        Out::error("Failed to open scene: %s", e.what());
        Backend::getApp().registerAlert("Error", "Failed to open scene file!");
    }
}

void Editor::Project::closeScene(uint32_t sceneId) {
    if (scenes.size() == 1) {
        Out::error("Cannot close last scene");
        return;
    }

    auto it = std::find_if(scenes.begin(), scenes.end(),
        [sceneId](const SceneProject& scene) { return scene.id == sceneId; });
    
    if (it != scenes.end()) {
        if (selectedScene == sceneId) {
            Out::error("Scene is selected, cannot close it");
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

void Editor::Project::resetConfigs() {
    // Clear existing scenes
    for (auto& sceneProject : scenes) {
        deleteSceneProject(&sceneProject);
    }
    scenes.clear();
    Backend::getApp().resetLastActivatedScene();

    // Reset state
    name = "";
    windowWidth = 1280;
    windowHeight = 720;
    selectedScene = NULL_PROJECT_SCENE;
    nextSceneId = 0;
    projectPath.clear();
    sharedGroups.clear();

    Backend::updateWindowTitle(name);

    //createNewScene("New Scene");
}

size_t Editor::Project::countEntitiesInBranch(const YAML::Node& entityNode) {
    size_t count = 1; // Count the entity itself

    if (entityNode["children"]) {
        for (const auto& child : entityNode["children"]) {
            count += countEntitiesInBranch(child);
        }
    }

    return count;
}

void Editor::Project::insertNewChild(YAML::Node& node, YAML::Node child, size_t index){
    if (node["children"]){
        size_t childrenSize = node["children"].size();

        // To insert at position i, we need to rebuild the sequence
        YAML::Node tempChildren = YAML::Node(YAML::NodeType::Sequence);

        // Copy elements before position i
        for (size_t j = 0; j < index && j < childrenSize; j++) {
            tempChildren.push_back(node["children"][j]);
        }

        // Insert the new child
        tempChildren.push_back(child);

        // Copy remaining elements
        for (size_t j = index; j < childrenSize; j++) {
            tempChildren.push_back(node["children"][j]);
        }

        node["children"] = tempChildren;
    }else{
        node["children"] = YAML::Node(YAML::NodeType::Sequence);
        node["children"].push_back(child);
    }
}

// Collect unique script source files from all scenes/entities
std::vector<Editor::ScriptSource> Editor::Project::collectScriptSourceFiles() const {
    std::unordered_set<std::string> uniqueScripts;
    std::vector<Editor::ScriptSource> scriptFiles;

    for (const auto& sceneProject : scenes) {
        Scene* scene = sceneProject.scene;
        for (Entity entity : sceneProject.entities) {
            Signature signature = scene->getSignature(entity);
            if (signature.test(scene->getComponentId<ScriptComponent>())) {
                const ScriptComponent& scriptComponent = scene->getComponent<ScriptComponent>(entity);

                // Iterate through all scripts in the component
                for (const auto& scriptEntry : scriptComponent.scripts) {
                    if (!scriptEntry.path.empty()) {
                        fs::path path = scriptEntry.path;
                        if (path.is_relative()) {
                            path = getProjectPath() / path;
                        }
                        fs::path headerPath = scriptEntry.headerPath;
                        if (headerPath.is_relative()) {
                            headerPath = getProjectPath() / headerPath;
                        }
                        if (std::filesystem::exists(path)) {
                            std::string key = path.lexically_normal().generic_string();
                            if (uniqueScripts.insert(key).second) {
                                scriptFiles.push_back(Editor::ScriptSource{path, headerPath, scriptEntry.className, sceneProject.scene, entity});
                            }
                        } else {
                            Out::error("Script file not found: %s", path.string().c_str());
                        }
                    }
                }
            }
        }
    }

    return scriptFiles;
}

void Editor::Project::pauseEngineScene(SceneProject* sceneProject, bool pause){
    sceneProject->scene->getSystem<PhysicsSystem>()->setPaused(pause);
    sceneProject->scene->getSystem<ActionSystem>()->setPaused(pause);
    sceneProject->scene->getSystem<AudioSystem>()->setPaused(pause);
}

void Editor::Project::copyEngineApiToProject() {
    try {
        std::filesystem::path exePath;
        #ifdef _WIN32
            char path[MAX_PATH];
            GetModuleFileNameA(NULL, path, MAX_PATH);
            exePath = std::filesystem::path(path).parent_path();
        #else
            exePath = std::filesystem::canonical("/proc/self/exe").parent_path();
        #endif
        std::filesystem::path engineApiSource = exePath / "engine-api";

        if (!std::filesystem::exists(engineApiSource)) {
            Out::warning("engine-api folder not found at: %s", engineApiSource.string().c_str());
            return;
        }

        std::filesystem::path engineApiDest = getProjectInternalPath() / "engine-api";

        // Create internal path if it doesn't exist
        if (!std::filesystem::exists(getProjectInternalPath())) {
            std::filesystem::create_directories(getProjectInternalPath());
        }

        // Copy with update_existing - only copies files that are newer
        std::filesystem::copy(engineApiSource, engineApiDest, 
                            std::filesystem::copy_options::recursive | 
                            std::filesystem::copy_options::update_existing);

        Out::info("Updated engine-api in project: %s", engineApiDest.string().c_str());

    } catch (const std::exception& e) {
        Out::error("Failed to copy engine-api: %s", e.what());
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
            Out::info("Created project directory: \"%s\"", projectPath.string().c_str());
            saveProject();
            createNewScene("New Scene", SceneType::SCENE_3D);
            copyEngineApiToProject();
        } else {
            Out::info("Project directory already exists: \"%s\"", projectPath.string().c_str());
            loadProject(projectPath);
        }

        Backend::getApp().updateResourcesPath();

    } catch (const std::exception& e) {
        printf("Error: %s\n", e.what());
        return false;
    }

    return true;
}

bool Editor::Project::saveProject(bool userCalled, std::function<void()> callback) {
    if (isTempProject() && userCalled) {
        Backend::getApp().registerProjectSaveDialog(callback);
        return true;
    }

    bool saveret = saveProjectToPath(projectPath);

    if (callback) {
        callback();
    }

    return saveret;
}

bool Editor::Project::saveProjectToPath(const std::filesystem::path& path) {
    // Try to create the directory if it doesn't exist
    if (!std::filesystem::exists(path)) {
        try {
            std::filesystem::create_directory(path);
        } catch (const std::exception& e) {
            Out::error("Failed to create project directory: %s", e.what());
            Backend::getApp().registerAlert("Error", "Failed to create project directory!");
            return false;
        }
    }

    // Check if we're moving from temp location
    bool wasTemp = isTempProject();
    std::filesystem::path oldPath = projectPath;

    projectPath = path;

    // If we're moving from a temp path, handle the file transfers
    if (wasTemp && oldPath != path) {
        try {
            // Copy all project files from temp dir to the new location
            for (const auto& entry : std::filesystem::directory_iterator(oldPath)) {
                std::filesystem::path destPath = path / entry.path().filename();
                std::filesystem::copy(entry.path(), destPath, 
                                     std::filesystem::copy_options::recursive);
            }

            // Update all scene filepaths to the new location
            for (auto& sceneProject : scenes) {
                if (!sceneProject.filepath.empty()) {
                    std::filesystem::path relativePath = std::filesystem::relative(
                        sceneProject.filepath, oldPath);
                    sceneProject.filepath = path / relativePath;
                }
            }

            // Delete the temp directory after moving all files
            std::filesystem::remove_all(oldPath);

        } catch (const std::exception& e) {
            Out::error("Failed to move project files: %s", e.what());
            Backend::getApp().registerAlert("Error", "Failed to move project files to the new location!");
            return false;
        }
    }

    // Now save the project file
    try {
        YAML::Node root = Stream::encodeProject(this);

        std::filesystem::path projectFile = path / "project.yaml";
        std::ofstream fout(projectFile.string());
        if (!fout) {
            Out::error("Failed to open project file for writing: %s", projectFile.string().c_str());
            return false;
        }

        fout << YAML::Dump(root);
        fout.close();

        // Update the app settings
        if (!isTempProject()){
            AppSettings::setLastProjectPath(path);
        }
        Backend::getApp().updateResourcesPath();

        return true;
    } catch (const std::exception& e) {
        Out::error("Failed to save project: \"%s\"", e.what());
        Backend::getApp().registerAlert("Error", "Failed to save project!");
        return false;
    }
}

bool Editor::Project::loadProject(const std::filesystem::path path) {
    resetConfigs();

    projectPath = path;

    try {
        if (!std::filesystem::exists(projectPath)) {
            Out::error("Project directory does not exist: \"%s\"", projectPath.string().c_str());
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
            createNewScene("New Scene", SceneType::SCENE_3D);
        }

        // Copy engine-api to project
        copyEngineApiToProject();

        Backend::getApp().updateResourcesPath();

        // Save this as the last opened project
        if (!isTempProject()) {
            AppSettings::setLastProjectPath(projectPath);
        }

        Out::info("Project loaded successfully: \"%s\"", projectPath.string().c_str());
        return true;

    } catch (const YAML::Exception& e) {
        Out::error("Failed to load project YAML: \"%s\"", e.what());
        Backend::getApp().registerAlert("Error", "Failed to load project file!");
        return false;
    } catch (const std::exception& e) {
        Out::error("Failed to load project: \"%s\"", e.what());
        Backend::getApp().registerAlert("Error", "Failed to load project!");
        return false;
    }
}

bool Editor::Project::openProject() {
    // Get user's home directory as default path
    std::string homeDirPath;
    #ifdef _WIN32
    homeDirPath = std::filesystem::path(getenv("USERPROFILE")).string();
    #else
    homeDirPath = std::filesystem::path(getenv("HOME")).string();
    #endif

    // Open a folder selection dialog
    std::string selectedDir = FileDialogs::openFileDialog(homeDirPath, false, true);

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
        Out::error("Failed to open project: \"%s\"", projectDir.string().c_str());
        Backend::getApp().registerAlert("Error", "Failed to open project!");
        return false;
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

    // Check if this scene has entities in shared groups and save them first
    for (const auto& [filepath, group] : sharedGroups) {
        if (group.instances.find(sceneId) != group.instances.end()) {
            saveSharedGroupToDisk(filepath);
            break;
        }
    }

    YAML::Node root = Stream::encodeSceneProject(this, sceneProject);
    std::ofstream fout(path.string());
    fout << YAML::Dump(root);
    fout.close();

    sceneProject->filepath = path;
    sceneProject->isModified = false;
    saveProject();

    Out::info("Scene saved to: \"%s\"", path.string().c_str());
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

Entity Editor::Project::findObjectByRay(uint32_t sceneId, float x, float y){
    SceneProject* scenedata = getScene(sceneId);
    Ray ray = scenedata->sceneRender->getCamera()->screenToRay(x, y);

    float distance = FLT_MAX;
    size_t index = 0;
    Entity selEntity = NULL_ENTITY;
    for (auto& entity : scenedata->entities) {
        AABB aabb;
        Signature signature = scenedata->scene->getSignature(entity);

        if (!signature.test(scenedata->scene->getComponentId<Transform>())){
            continue;
        }

        if (signature.test(scenedata->scene->getComponentId<MeshComponent>())){
            MeshComponent& mesh = scenedata->scene->getComponent<MeshComponent>(entity);
            aabb = mesh.worldAABB;
        }else if (signature.test(scenedata->scene->getComponentId<UIComponent>())){
            UIComponent& ui = scenedata->scene->getComponent<UIComponent>(entity);
            aabb = ui.worldAABB;
        }else if (signature.test(scenedata->scene->getComponentId<LightComponent>())){
            Transform& transform = scenedata->scene->getComponent<Transform>(entity);
            Transform& camtransform = scenedata->scene->getComponent<Transform>(scenedata->scene->getCamera());
            CameraComponent& camera = scenedata->scene->getComponent<CameraComponent>(scenedata->scene->getCamera());
            float dist = (transform.worldPosition - camtransform.worldPosition).length();
            float size = dist * tan(camera.yfov) * 0.01;
            aabb = transform.modelMatrix * AABB(-size, -size, -size, size, size, size);
        }

        if (!aabb.isNull() && !aabb.isInfinite()){
            RayReturn rreturn = ray.intersects(aabb);
            if (rreturn.hit){
                size_t nIndex = scenedata->scene->getComponentArray<Transform>()->getIndex(entity);
                if (rreturn.distance < distance || (nIndex >= index && scenedata->sceneType != SceneType::SCENE_3D)){
                    distance = rreturn.distance;
                    index = nIndex;
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

    if (!scenedata->sceneRender->isAnyGizmoSideSelected()){
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

        AABB aabb;
        Signature signature = scenedata->scene->getSignature(entity);

        if (!signature.test(scenedata->scene->getComponentId<Transform>())){
            continue;
        }

        if (signature.test(scenedata->scene->getComponentId<MeshComponent>())){
            MeshComponent& mesh = scenedata->scene->getComponent<MeshComponent>(entity);
            aabb = mesh.aabb;
        }else if (signature.test(scenedata->scene->getComponentId<UIComponent>())){
            UIComponent& ui = scenedata->scene->getComponent<UIComponent>(entity);
            aabb = ui.aabb;
        }else if (signature.test(scenedata->scene->getComponentId<LightComponent>())){
            Transform& transform = scenedata->scene->getComponent<Transform>(entity);
            aabb = AABB::ZERO; // just a point
        }

        if (!aabb.isNull() && !aabb.isInfinite()){
            Transform& transform = scenedata->scene->getComponent<Transform>(entity);

            const Vector3* corners = aabb.getCorners();

            Vector2 minRect = Vector2(std::min(start.x, end.x), std::min(start.y, end.y));
            Vector2 maxRect = Vector2(std::max(start.x, end.x), std::max(start.y, end.y));

            bool found = true;

            for (int c = 0; c < 8; c++){
                Vector4 clipCorner = camera->getViewProjectionMatrix() * transform.modelMatrix * Vector4(corners[c], 1.0);
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

        //debugSceneHierarchy();
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

std::filesystem::path Editor::Project::getProjectInternalPath() const{
    return projectPath / ".supernova";
}

fs::path Editor::Project::getThumbsDir() const{
    return getProjectInternalPath() / "thumbs";
}

fs::path Editor::Project::getThumbnailPath(const fs::path& originalPath) const {
    fs::path thumbsDir = getThumbsDir();

    // Get relative path from project root, as a string
    fs::path relativePath = fs::relative(originalPath, getProjectPath());
    std::string relPathStr = relativePath.generic_string();

    // Include file size and modification time in hash for uniqueness
    auto fileSize = fs::file_size(originalPath);
    auto modTime = fs::last_write_time(originalPath).time_since_epoch().count();
    std::string hashInput = relPathStr + "_" + std::to_string(static_cast<uint64_t>(fileSize)) + "_" + std::to_string(static_cast<int64_t>(modTime));

    // Hash the combined string
    std::string hash = SHA1::hash(hashInput);

    std::string thumbFilename = hash + ".thumb.png";
    return thumbsDir / thumbFilename;
}

std::vector<Entity> Editor::Project::getEntities(uint32_t sceneId) const{
    return getScene(sceneId)->entities;
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

    for (const auto& [filepath, group] : sharedGroups) {
        if (group.isModified) {
            return true;
        }
    }

    return false;
}

void Editor::Project::updateAllScriptsProperties(uint32_t sceneId){
    SceneProject* sceneProject = getScene(sceneId);
    if (!sceneProject) return;

    for (Entity entity : sceneProject->entities) {
        Signature signature = sceneProject->scene->getSignature(entity);
        if (signature.test(sceneProject->scene->getComponentId<ScriptComponent>())) {
            ScriptComponent& scriptComponent = sceneProject->scene->getComponent<ScriptComponent>(entity);
            updateScriptProperties(sceneProject, scriptComponent.scripts);
        }
    }
}

void Editor::Project::updateScriptProperties(SceneProject* sceneProject, std::vector<ScriptEntry>& scripts){
    bool hasChanges = false;

    // Update properties for each script in the component
    for (auto& scriptEntry : scripts) {
        // Parse the script file to extract properties
        fs::path fullPath = scriptEntry.headerPath;
        if (fullPath.is_relative()) {
            fullPath = getProjectPath() / fullPath;
        }

        std::vector<ScriptProperty> parsedProperties = ScriptParser::parseScriptProperties(fullPath);

        if (parsedProperties.empty()) {
            // No properties found or parsing failed
            if (!scriptEntry.properties.empty()) {
                scriptEntry.properties.clear();
                hasChanges = true;
            }
            continue;
        }

        // Merge with existing properties to preserve user-modified values
        std::vector<ScriptProperty> mergedProperties;
        bool structuralChanges = false; // added/removed properties
        bool metaChanges = false;       // display name/type changed

        for (const auto& parsedProp : parsedProperties) {
            // Try to find existing property with same internal name
            auto it = std::find_if(scriptEntry.properties.begin(), scriptEntry.properties.end(),
                [&parsedProp](const ScriptProperty& existing) {
                    return existing.name == parsedProp.name;
                });

            if (it != scriptEntry.properties.end()) {
                // Property exists - keep user's value but update metadata
                ScriptProperty merged = parsedProp;

                // If the type changed, reset the value to the new default to avoid invalid variants
                if (it->type == parsedProp.type) {
                    merged.value = it->value;
                } else {
                    merged.value = parsedProp.defaultValue;
                    metaChanges = true;
                }

                // Preserve member pointer
                merged.memberPtr = it->memberPtr;

                // Detect metadata changes (no structural change)
                if (it->displayName != parsedProp.displayName ||
                    it->type != parsedProp.type ||
                    it->ptrTypeName != parsedProp.ptrTypeName) {
                    metaChanges = true;
                }

                mergedProperties.push_back(std::move(merged));
            } else {
                // New property - use default value
                mergedProperties.push_back(parsedProp);
                structuralChanges = true;
            }
        }

        // Check if any properties were removed
        if (scriptEntry.properties.size() != mergedProperties.size()) {
            structuralChanges = true;
        }

        // Always apply merged properties so UI metadata updates (e.g., displayName) are reflected
        scriptEntry.properties = std::move(mergedProperties);

        if (structuralChanges || metaChanges) {
            hasChanges = true;
        }
    }

    if (hasChanges) {
        sceneProject->isModified = true;
    }
}

bool Editor::Project::markEntityShared(uint32_t sceneId, Entity entity, fs::path filepath, YAML::Node entityNode){
    if (!filepath.is_relative()) {
        Out::error("Shared entity filepath must be relative: %s", filepath.string().c_str());
        return false;
    }

    auto it = sharedGroups.find(filepath);
    if (it != sharedGroups.end()) {
        Out::error("Shared entity group already exists at %s", filepath.string().c_str());
        return false;
    }

    // Get all entities in the branch (root + children)
    SceneProject* sceneProject = getScene(sceneId);
    if (!sceneProject) {
        return false;
    }

    std::vector<Entity> branchEntities;
    collectEntities(entityNode, branchEntities);

    // Create new group
    SharedGroup newGroup;
    newGroup.registry = std::make_unique<EntityRegistry>();
    newGroup.isModified = true;

    SharedGroup::Instance newInstance;
    newInstance.instanceId = newGroup.nextInstanceId++;

    std::vector<Entity> regEntities = Stream::decodeEntity(clearEntitiesNode(entityNode), newGroup.registry.get(), &newGroup.registryEntities);
    for (int i = 0; i < regEntities.size(); i++) {
        newInstance.members.push_back({branchEntities[i], regEntities[i]});
    }

    Scene* scene = sceneProject->scene;

    // Mark Transform component of root entity as overridden for this scene
    // This allows each scene to position the shared entity independently
    if (scene->findComponent<Transform>(entity)) {
        newInstance.overrides[entity] = 1ULL << static_cast<int>(ComponentType::Transform);

        newGroup.registry->getComponent<Transform>(NULL_ENTITY + 1) = {};
    }

    newGroup.instances[sceneId].push_back(std::move(newInstance));

    sharedGroups.emplace(filepath, std::move(newGroup));

    // Set up event subscriptions for this shared group
    saveSharedGroupToDisk(filepath);

    sceneProject->isModified = true;

    return true;
}

bool Editor::Project::removeSharedGroup(const std::filesystem::path& filepath) {
    auto it = sharedGroups.find(filepath);
    if (it != sharedGroups.end()) {
        // Clear the shared group's registry
        if (it->second.registry) {
            it->second.registry->clear();
        }

        // Remove from the map
        sharedGroups.erase(it);

        Editor::Out::info("Removed shared group: %s", filepath.string().c_str());
        return true;
    }
    return false;
}

std::vector<Entity> Editor::Project::importSharedEntity(SceneProject* sceneProject, std::vector<Entity>* entities, const std::filesystem::path& filepath, Entity parent, bool needSaveScene, YAML::Node extendNode) {
    if (!filepath.is_relative()) {
        Out::error("Shared entity filepath must be relative: %s", filepath.string().c_str());
        return {};
    }

    auto it = sharedGroups.find(filepath);

    if (it == sharedGroups.end()) {
        // Entity doesn't exist in any scene yet - create new SharedGroup
        SharedGroup newGroup;
        newGroup.registry = std::make_unique<EntityRegistry>();
        newGroup.isModified = false;

        auto [newIt, inserted] = sharedGroups.emplace(filepath, std::move(newGroup));
        it = newIt;
    }

    auto& group = it->second;

    SharedGroup::Instance newInstance;
    newInstance.instanceId = group.nextInstanceId++;

    YAML::Node node;
    // Use registry if modified, otherwise load from file
    if (group.isModified && (group.registry->getLastEntity() > NULL_ENTITY)) {
        node = Stream::encodeEntity(NULL_ENTITY + 1, group.registry.get());
    } else {
        try {
            std::filesystem::path fullSharedPath = getProjectPath() / filepath;
            node = YAML::LoadFile(fullSharedPath.string());
            group.registry->clear();
            Stream::decodeEntity(node, group.registry.get(), &group.registryEntities);
        } catch (const YAML::Exception& e) {
            Out::error("Failed to load shared entity file: %s", e.what());
            return {};
        } catch (const std::exception& e) {
            Out::error("Failed to load shared entity file: %s", e.what());
            return {};
        }
    }

    // Merge extendNode with loadedNode if extendNode is provided
    std::vector<MergeResult> mergeResults;
    if (extendNode && !extendNode.IsNull()) {
        mergeResults = mergeEntityNodes(extendNode, node);
    }

    // decode into brand‐new local entities (root + children)
    Scene* scene = sceneProject->scene;
    std::vector<Entity> newEntities = Stream::decodeEntity(node, scene, entities, this, sceneProject, parent, false);
    scene->addEntityChild(parent, newEntities[0], false);

    std::vector<Entity> membersEntities;
    if (mergeResults.empty()){
        membersEntities = newEntities;
        newInstance.overrides[newEntities[0]] = 1ULL << static_cast<int>(ComponentType::Transform);
    }else{
        for (int i = 0; i < newEntities.size(); i++) {
            if (i >= mergeResults.size()){
                // When shared entity is updated (saved) and scene has not been updated
                membersEntities.push_back(newEntities[i]);
            }else if (mergeResults[i].isShared){
                membersEntities.push_back(newEntities[i]);
                newInstance.overrides[newEntities[i]] = mergeResults[i].overrides;
            }
        }
    }

    std::vector<Entity> regEntities = group.registry->getEntityList();

    if (membersEntities.size() != regEntities.size()) {
        Out::error("Mismatch in shared entity count when importing from %s", filepath.string().c_str());
        return {};
    }

    for (int i = 0; i < membersEntities.size(); i++) {
        newInstance.members.push_back({membersEntities[i], regEntities[i]});
    }

    // Mark Transform component of root entity as overridden for this scene
    // This allows each scene to position the shared entity independently
    Entity rootEntity = newEntities[0];
    if (scene->findComponent<Transform>(rootEntity)) {
        group.setComponentOverride(sceneProject->id, rootEntity, ComponentType::Transform);
    }

    group.instances[sceneProject->id].push_back(std::move(newInstance));

    sceneProject->isModified = needSaveScene;

    return newEntities;
}

bool Editor::Project::unimportSharedEntity(uint32_t sceneId, const std::filesystem::path& filepath, const std::vector<Entity>& entities, bool destroyEntities) {
    SceneProject* sceneProject = getScene(sceneId);
    if (!sceneProject) {
        return false;
    }

    Scene* scene = sceneProject->scene;

    // Remove from shared group
    SharedGroup* group = getSharedGroup(filepath);
    if (group) {
        // Find which instance these entities belong to
        uint32_t instanceId = 0;
        if (!entities.empty()) {
            instanceId = group->getInstanceId(sceneId, entities[0]);
        }

        if (instanceId != 0) {
            // Remove only this specific instance
            group->removeInstance(sceneId, instanceId);

            // Mark as modified if group still has instances
            if (group->getTotalInstanceCount() > 0) {
                group->isModified = true;
            }
        } else {
            Out::error("Could not find instance for entities in scene %u", sceneId);
            return false;
        }
    }

    if (destroyEntities){
        // Destroy all imported entities
        for (Entity entity : entities) {
            DeleteEntityCmd::destroyEntity(scene, entity, sceneProject->entities, this, sceneId);
        }
    }

    sceneProject->isModified = true;

    return true;
}

bool Editor::Project::addEntityToSharedGroup(uint32_t sceneId, Entity entity, Entity parent, bool createItself){
    SceneProject* sceneProject = getScene(sceneId);
    if (!sceneProject){
        return false;
    }

    Scene* scene = sceneProject->scene;

    // Find which shared group and instance the parent belongs to
    fs::path filepath = findGroupPathFor(sceneId, parent);
    if (filepath.empty()) {
        Out::error("Entity parent %u in scene %u is not part of any shared group", parent, sceneId);
        return false;
    }

    SharedGroup* group = getSharedGroup(filepath);
    uint32_t instanceId = group->getInstanceId(sceneId, parent);

    // Find the insertion position relative to other shared entities
    Entity beforeSharedEntity = NULL_ENTITY;
    Entity afterSharedEntity = NULL_ENTITY;

    std::vector<Entity> instanceEntities = group->getAllEntities(sceneId, instanceId);

    // Find the entity in the entities list
    auto entityIt = std::find(sceneProject->entities.begin(), sceneProject->entities.end(), entity);
    if (entityIt != sceneProject->entities.end()) {
        // Look backwards for a shared entity
        for (auto it = entityIt - 1; it != sceneProject->entities.begin(); --it) {
            if (std::find(instanceEntities.begin(), instanceEntities.end(), *it) != instanceEntities.end()) {
                beforeSharedEntity = *it;
                break;
            }
        }
        // Look forwards for a shared entity
        for (auto it = entityIt + 1; it != sceneProject->entities.end(); ++it) {
            if (std::find(instanceEntities.begin(), instanceEntities.end(), *it) != instanceEntities.end()) {
                afterSharedEntity = *it;
                break;
            }
        }
    }

    Signature signature = scene->getSignature(entity);

    if (beforeSharedEntity != NULL_ENTITY){
        Signature signatureB = scene->getSignature(beforeSharedEntity);
        if (signature.test(scene->getComponentId<Transform>())){
            if (signatureB.test(scene->getComponentId<Transform>())){
                Transform& transform = scene->getComponent<Transform>(entity);
                Transform& transformB = scene->getComponent<Transform>(beforeSharedEntity);
                if (transform.parent != transformB.parent){
                    beforeSharedEntity = NULL_ENTITY;
                }
            }else{
                beforeSharedEntity = NULL_ENTITY;
            }
        }
    }
    if (afterSharedEntity != NULL_ENTITY){
        Signature signatureA = scene->getSignature(afterSharedEntity);
        if (signature.test(scene->getComponentId<Transform>())){
            if (signatureA.test(scene->getComponentId<Transform>())){
                Transform& transform = scene->getComponent<Transform>(entity);
                Transform& transformA = scene->getComponent<Transform>(afterSharedEntity);
                if (transform.parent != transformA.parent){
                    afterSharedEntity = NULL_ENTITY;
                }
            }else{
                afterSharedEntity = NULL_ENTITY;
            }
        }
    }

    NodeRecovery entityData;

    std::string recoveryKey = std::to_string(sceneId) + "_" + std::to_string(instanceId);
    entityData[recoveryKey].node = Stream::encodeEntity(entity, scene, nullptr, sceneProject, true);
    //entityData[sceneId].transformIndex = ProjectUtils::getTransformIndex(scene, entity);

    if (addEntityToSharedGroup(sceneId, entityData, parent, instanceId, createItself)){

        if (beforeSharedEntity != NULL_ENTITY){
            moveEntityFromSharedGroup(sceneId, entity, beforeSharedEntity, InsertionType::AFTER, false);
        }else if (afterSharedEntity != NULL_ENTITY){
            moveEntityFromSharedGroup(sceneId, entity, afterSharedEntity, InsertionType::BEFORE, false);
        }

        return true;
    }

    return false;
}

bool Editor::Project::addEntityToSharedGroup(uint32_t sceneId, const Editor::NodeRecovery& recoveryData, Entity parent, uint32_t instanceId, bool createItself){
    fs::path filepath = findGroupPathFor(sceneId, parent);
    if (filepath.empty()) {
        Out::error("Entity parent %u in scene %u is not part of any shared group", parent, sceneId);
        return {};
    }

    SharedGroup* group = getSharedGroup(filepath);

    SceneProject* sceneProject = getScene(sceneId);
    if (!sceneProject) {
        return false;
    }

    // If instanceId is not provided (0), find it from the parent
    if (instanceId == 0) {
        instanceId = group->getInstanceId(sceneId, parent);
        if (instanceId == 0) {
            Out::error("Failed to find instance for parent entity %u in scene %u", parent, sceneId);
            return false;
        }
    }

    Entity registryParent = group->getRegistryEntity(sceneId, parent);
    if (registryParent == NULL_ENTITY) {
        Out::error("Failed to find registry entity for shared entity %u in scene %u", parent, sceneId);
        return false;
    }

    YAML::Node nodeRegData;
    size_t regTransformIndex = 0;
    bool hasRegRecoveryData = false;
    std::string recoveryDefKey = std::to_string(NULL_PROJECT_SCENE);
    if (recoveryData.find(recoveryDefKey) != recoveryData.end()) {
        hasRegRecoveryData = true;
        nodeRegData = recoveryData.at(recoveryDefKey).node;
        regTransformIndex = recoveryData.at(recoveryDefKey).transformIndex;
    }else{
        std::string recoveryKey = std::to_string(sceneId) + "_" + std::to_string(instanceId);
        if (recoveryData.find(recoveryKey) == recoveryData.end()) {
            Out::error("No default entity data provided for adding to shared group");
            return false;
        }else{
            nodeRegData = clearEntitiesNode(YAML::Clone(recoveryData.at(recoveryKey).node));
        }
    }

    std::vector<Entity> regEntities =  Stream::decodeEntity(nodeRegData, group->registry.get(), &group->registryEntities);
    ProjectUtils::moveEntityOrderByTransform(group->registry.get(), group->registryEntities, regEntities[0], registryParent, regTransformIndex, hasRegRecoveryData);

    for (auto& [otherSceneId, sceneInstances] : group->instances) {
        for (auto& instance : sceneInstances) {
            std::string recoveryKey = std::to_string(otherSceneId) + "_" + std::to_string(instance.instanceId);
            YAML::Node nodeData;
            size_t transformIndex = 0;
            std::vector<MergeResult> mergeResults;
            bool hasRecoveryData = false;
            if (recoveryData.find(recoveryKey) != recoveryData.end()) {
                hasRecoveryData = true;
                nodeData = recoveryData.at(recoveryKey).node;
                transformIndex = recoveryData.at(recoveryKey).transformIndex;
                mergeResults = recoveryData.at(recoveryKey).mergeResults;
            } else {
                nodeData = Stream::encodeEntity(regEntities[0], group->registry.get());
            }

            SceneProject* otherScene = getScene(otherSceneId);
            if (!otherScene) {
                Out::error("Failed to find scene %u", otherSceneId);
                continue;
            }

            std::vector<Entity> newOtherEntities;

            if ((otherSceneId != sceneId) || (instance.instanceId != instanceId) || createItself) {
                // Find the parent in this instance
                Entity otherParent = group->getLocalEntity(otherSceneId, instance.instanceId, registryParent);
                if (otherParent == NULL_ENTITY) {
                    Out::error("Failed to find parent entity in scene %u instance %u", otherSceneId, instance.instanceId);
                    continue;
                }

                newOtherEntities = Stream::decodeEntity(nodeData, otherScene->scene, &otherScene->entities);
                ProjectUtils::moveEntityOrderByTransform(otherScene->scene, otherScene->entities, newOtherEntities[0], otherParent, transformIndex, hasRecoveryData);
            } else {
                // Just collect the entities from the node without creating them
                collectEntities(nodeData, newOtherEntities);
            }

            // Process merge results and add entities to the instance
            std::vector<Entity> membersEntities;
            if (mergeResults.empty()) {
                membersEntities = newOtherEntities;
            } else {
                for (int i = 0; i < newOtherEntities.size(); i++) {
                    if (mergeResults[i].isShared) {
                        membersEntities.push_back(newOtherEntities[i]);
                        instance.overrides[newOtherEntities[i]] = mergeResults[i].overrides;
                    }
                }
            }

            if (regEntities.size() != membersEntities.size()) {
                Out::error("Mismatch in shared entity count when adding to shared group %s", filepath.string().c_str());
                return false;
            }

            // Add the new entity members to this instance
            for (int e = 0; e < membersEntities.size(); e++) {
                instance.members.push_back({membersEntities[e], regEntities[e]});
            }

            otherScene->isModified = true;
        }
    }

    group->isModified = true;

    return true;
}

Editor::NodeRecovery Editor::Project::removeEntityFromSharedGroup(uint32_t sceneId, Entity entity, bool destroyItself) {
    fs::path filepath = findGroupPathFor(sceneId, entity);
    if (filepath.empty()) {
        Out::error("Entity %u in scene %u is not part of any shared group", entity, sceneId);
        return {};
    }

    SharedGroup* group = getSharedGroup(filepath);
    uint32_t instanceId = group->getInstanceId(sceneId, entity);

    SceneProject* sceneProject = getScene(sceneId);
    if (!sceneProject) {
        return {};
    }

    Entity registryEntity = group->getRegistryEntity(sceneId, entity);
    if (registryEntity == NULL_ENTITY) {
        Out::error("Failed to find registry entity for shared entity %u in scene %u", entity, sceneId);
        return {};
    }
    YAML::Node regData = Stream::encodeEntity(registryEntity, group->registry.get(), nullptr, nullptr, true);

    size_t transformIndex;
    NodeRecovery recovery;

    transformIndex = ProjectUtils::getTransformIndex(group->registry.get(), registryEntity);
    std::string recoveryDefKey = std::to_string(NULL_PROJECT_SCENE);
    recovery[recoveryDefKey] = {YAML::Clone(regData), std::vector<MergeResult>{}, transformIndex};

    // Process each scene that has instances
    for (auto& [otherSceneId, sceneInstances] : group->instances) {
        // Inverting to get correct transformIndex for addEntity
        for (auto it = sceneInstances.rbegin(); it != sceneInstances.rend(); ++it) {
            auto& instance = *it;
            Entity otherEntity = group->getLocalEntity(otherSceneId, instance.instanceId, registryEntity);
            SceneProject* otherScene = getScene(otherSceneId);
            if (!otherScene) {
                Out::error("Failed to find scene %u", otherSceneId);
                continue;
            }

            YAML::Node nodeExtend = Stream::encodeEntity(otherEntity, otherScene->scene, this, otherScene, true);

            std::vector<Entity> allEntities;
            std::vector<Entity> sharedEntities;
            collectEntities(nodeExtend, allEntities, sharedEntities);

            // Merge with registry data to capture overrides
            YAML::Node node = YAML::Clone(regData);
            std::vector<MergeResult> mergeResults = mergeEntityNodes(nodeExtend, node);

            transformIndex = ProjectUtils::getTransformIndex(otherScene->scene, otherEntity);

            std::string recoveryKey = std::to_string(otherSceneId) + "_" + std::to_string(instance.instanceId);
            recovery[recoveryKey] = {node, mergeResults, transformIndex};

            // Remove entities from this instance
            for (const Entity& sharedE : sharedEntities) {
                // Find and remove the EntityMember from this instance
                auto itRem = std::find_if(instance.members.begin(), instance.members.end(),
                    [sharedE](const SharedGroup::EntityMember& member) {
                        return member.localEntity == sharedE;
                    });
                if (itRem != instance.members.end()) {
                    instance.members.erase(itRem);
                }

                // Clear overrides for this entity in this instance
                instance.overrides.erase(sharedE);
            }

            // Destroy the entities if needed
            if ((otherSceneId != sceneId) || (instance.instanceId != instanceId) || destroyItself) {
                for (const Entity& entityToDestroy : allEntities) {
                    DeleteEntityCmd::destroyEntity(otherScene->scene, entityToDestroy, otherScene->entities, this, otherSceneId);
                }
            }

            otherScene->isModified = true;
        }

        // Clean up empty instances
        sceneInstances.erase(
            std::remove_if(sceneInstances.begin(), sceneInstances.end(),
                [](const SharedGroup::Instance& inst) { 
                    return inst.members.empty(); 
                }),
            sceneInstances.end()
        );
    }

    // Clean up scenes with no instances
    for (auto it = group->instances.begin(); it != group->instances.end(); ) {
        if (it->second.empty()) {
            it = group->instances.erase(it);
        } else {
            ++it;
        }
    }

    std::vector<Entity> registryEntitiesToRemove;
    collectEntities(regData, registryEntitiesToRemove);

    // Destroy entities from registry
    for (Entity regEntity : registryEntitiesToRemove) {
        DeleteEntityCmd::destroyEntity(group->registry.get(), regEntity, group->registryEntities);
    }

    group->isModified = true;

    return recovery;
}

Editor::SharedMoveRecovery Editor::Project::moveEntityFromSharedGroup(uint32_t sceneId, Entity entity, Entity target, InsertionType type, bool moveItself){
    fs::path filepath = findGroupPathFor(sceneId, entity);
    if (filepath.empty()) {
        Out::error("Entity %u in scene %u is not part of any shared group", entity, sceneId);
        return {};
    }

    SharedGroup* group = getSharedGroup(filepath);
    uint32_t instanceId = group->getInstanceId(sceneId, entity);

    if (!isEntityShared(sceneId, target)){
        if (type == InsertionType::IN){
            Out::error("Cannot move shared entity %u into non-shared target %u in scene %u", entity, target, sceneId);
            return {};
        }

        auto& entities = getScene(sceneId)->entities;
        auto entityIt = std::find(entities.begin(), entities.end(), entity);
        auto targetIt = std::find(entities.begin(), entities.end(), target);

        if (entityIt != entities.end() && targetIt != entities.end()) {
            Entity nextShared = NULL_ENTITY;

            if (entityIt < targetIt) {
                for (auto it = targetIt - 1; it > entityIt; --it) {
                    if (isEntityShared(sceneId, *it)) {
                        nextShared = *it;
                        break;
                    }
                }
            } else {
                for (auto it = targetIt + 1; it < entityIt; ++it) {
                    if (isEntityShared(sceneId, *it)) {
                        nextShared = *it;
                        break;
                    }
                }
            }

            if (nextShared != NULL_ENTITY) {
                target = nextShared;
            }else{
                // Not need to move entity in other scenes and registry if target is not shared
                return {};
            }
        }
    }

    Entity registryEntity = group->getRegistryEntity(sceneId, entity);
    Entity registryTarget = group->getRegistryEntity(sceneId, target);
    if (registryEntity == NULL_ENTITY || registryTarget == NULL_ENTITY) {
        Out::error("Failed to find registry entities for shared entities %u or %u in scene %u", entity, target, sceneId);
        return {};
    }

    SharedMoveRecovery recovery;

    Entity oldParent;
    size_t oldIndex;
    bool hasTransform;
    ProjectUtils::moveEntityOrderByTarget(group->registry.get(), group->registryEntities, registryEntity, registryTarget, type, oldParent, oldIndex, hasTransform);
    std::string recoveryDefKey = std::to_string(NULL_PROJECT_SCENE);
    recovery[recoveryDefKey] = {oldParent, oldIndex, hasTransform};

    for (auto& [otherSceneId, sceneInstances] : group->instances) {
        SceneProject* otherScene = getScene(otherSceneId);
        if (otherScene){
            for (auto& instance : sceneInstances) {
                if ((otherSceneId != sceneId) || (instance.instanceId != instanceId) || moveItself) {
                    Entity otherEntity = group->getLocalEntity(otherSceneId, instance.instanceId, registryEntity);
                    Entity otherTarget = group->getLocalEntity(otherSceneId, instance.instanceId, registryTarget);

                    if (otherEntity != NULL_ENTITY && otherTarget != NULL_ENTITY) {
                        Entity otherOldParent;
                        size_t otherOldIndex;
                        bool otherHasTransform;
                        ProjectUtils::moveEntityOrderByTarget(otherScene->scene, otherScene->entities, otherEntity, otherTarget, type, otherOldParent, otherOldIndex, otherHasTransform);
                        std::string recoveryKey = std::to_string(otherSceneId) + "_" + std::to_string(instance.instanceId);
                        recovery[recoveryKey] = {otherOldParent, otherOldIndex, otherHasTransform};

                        otherScene->isModified = true;
                    }
                }
            }
        }
    }

    return recovery;
}

bool Editor::Project::undoMoveEntityInSharedGroup(uint32_t sceneId, Entity entity, Entity target, const SharedMoveRecovery& recovery, bool moveItself){
    fs::path filepath = findGroupPathFor(sceneId, entity);
    if (filepath.empty()) {
        Out::error("Entity %u in scene %u is not part of any shared group", entity, sceneId);
        return false;
    }

    SharedGroup* group = getSharedGroup(filepath);
    uint32_t instanceId = group->getInstanceId(sceneId, entity);

    Entity registryEntity = group->getRegistryEntity(sceneId, entity);
    Entity registryTarget = group->getRegistryEntity(sceneId, target);
    if (registryEntity == NULL_ENTITY || registryTarget == NULL_ENTITY) {
        Out::error("Failed to find registry entities for shared entities %u or %u in scene %u", entity, target, sceneId);
        return {};
    }

    std::string recoveryDefKey = std::to_string(NULL_PROJECT_SCENE);
    if (recovery.find(recoveryDefKey) == recovery.end()) {
        Out::error("No recovery data provided for undoing move of entity %u in scene %u", entity, sceneId);
        return false;
    }
    ProjectUtils::moveEntityOrderByIndex(group->registry.get(), group->registryEntities, registryEntity, recovery.at(recoveryDefKey).oldParent, recovery.at(recoveryDefKey).oldIndex, recovery.at(recoveryDefKey).hasTransform);

    for (auto& [otherSceneId, sceneInstances] : group->instances) {
        SceneProject* otherScene = getScene(otherSceneId);
        if (otherScene) {
            // Inverting to get correct entity index
            for (auto it = sceneInstances.rbegin(); it != sceneInstances.rend(); ++it) {
                auto& instance = *it;
                if ((otherSceneId != sceneId) || (instance.instanceId != instanceId) || moveItself) {
                    std::string recoveryKey = std::to_string(otherSceneId) + "_" + std::to_string(instance.instanceId);
                    if (recovery.find(recoveryKey) != recovery.end()) {
                        Entity otherEntity = group->getLocalEntity(otherSceneId, instance.instanceId, registryEntity);
                        Entity otherTarget = group->getLocalEntity(otherSceneId, instance.instanceId, registryTarget);
                        if (otherEntity != NULL_ENTITY && otherTarget != NULL_ENTITY) {
                            ProjectUtils::moveEntityOrderByIndex(otherScene->scene, otherScene->entities, otherEntity, recovery.at(recoveryKey).oldParent, recovery.at(recoveryKey).oldIndex, recovery.at(recoveryKey).hasTransform);
                        }
                    }
                }
            }
        }
    }

    return true;
}

bool Editor::Project::addComponentToSharedGroup(uint32_t sceneId, Entity entity, ComponentType componentType, bool addToItself){
    ComponentRecovery recovery;
    return addComponentToSharedGroup(sceneId, entity, componentType, recovery, addToItself);
}

bool Editor::Project::addComponentToSharedGroup(uint32_t sceneId, Entity entity, ComponentType componentType, const ComponentRecovery& recovery, bool addToItself){
    fs::path filepath = findGroupPathFor(sceneId, entity);
    if (filepath.empty()) {
        Out::error("Entity %u in scene %u is not part of any shared group", entity, sceneId);
        return false;
    }

    SharedGroup* group = getSharedGroup(filepath);
    uint32_t instanceId = group->getInstanceId(sceneId, entity);

    if (group->hasComponentOverride(sceneId, entity, componentType)){
        Out::warning("Component %s of entity %u in scene %u is overridden", Catalog::getComponentName(componentType).c_str(), entity, sceneId);
        return false;
    }

    Entity registryEntity = group->getRegistryEntity(sceneId, entity);
    if (registryEntity == NULL_ENTITY) {
        Out::error("Failed to find registry entities for shared entities %u in scene %u", entity, sceneId);
        return false;
    }

    YAML::Node regNode;
    std::string recoveryDefKey = std::to_string(NULL_PROJECT_SCENE);
    if (recovery.find(recoveryDefKey) != recovery.end()) {
        if (recovery.at(recoveryDefKey).entity == registryEntity){
            regNode = recovery.at(recoveryDefKey).node;
        }else{
            Out::error("Component recovery entity (%u) does not match registry entity (%u)", recovery.at(recoveryDefKey).entity, registryEntity);
            return false;
        }
    }

    ProjectUtils::addEntityComponent(group->registry.get(), registryEntity, componentType, group->registryEntities, regNode);

    for (auto& [otherSceneId, sceneInstances] : group->instances) {
        SceneProject* otherScene = getScene(otherSceneId);
        if (otherScene){
            for (auto& instance : sceneInstances) {
                if ((otherSceneId != sceneId) || (instance.instanceId != instanceId) || addToItself) {
                    Entity otherEntity = group->getLocalEntity(otherSceneId, instance.instanceId, registryEntity);

                    if (otherEntity != NULL_ENTITY) {
                        if (!group->hasComponentOverride(otherSceneId, otherEntity, componentType)) {
                            YAML::Node compNode;
                            std::string recoveryKey = std::to_string(otherSceneId) + "_" + std::to_string(instance.instanceId);
                            if (recovery.find(recoveryKey) != recovery.end()) {
                                if (recovery.at(recoveryKey).entity == otherEntity){
                                    compNode = recovery.at(recoveryKey).node;
                                }else{
                                    Out::warning("Component recovery entity (%u) does not match scene (%u) entity (%u)", recovery.at(recoveryKey).entity, otherSceneId, otherEntity);
                                    return false;
                                }
                            }

                            ProjectUtils::addEntityComponent(otherScene->scene, otherEntity, componentType, otherScene->entities, compNode);

                            otherScene->isModified = true;
                        }
                    }
                }
            }
        }
    }

    group->isModified = true;

    return true;
}

Editor::ComponentRecovery Editor::Project::removeComponentToSharedGroup(uint32_t sceneId, Entity entity, ComponentType componentType, bool encodeComponent, bool removeToItself){
    fs::path filepath = findGroupPathFor(sceneId, entity);
    if (filepath.empty()) {
        Out::error("Entity %u in scene %u is not part of any shared group", entity, sceneId);
        return {};
    }

    SharedGroup* group = getSharedGroup(filepath);
    uint32_t instanceId = group->getInstanceId(sceneId, entity);

    if (group->hasComponentOverride(sceneId, entity, componentType)){
        Out::warning("Component %s of entity %u in scene %u is overridden", Catalog::getComponentName(componentType).c_str(), entity, sceneId);
        return {};
    }

    Entity registryEntity = group->getRegistryEntity(sceneId, entity);
    if (registryEntity == NULL_ENTITY) {
        Out::error("Failed to find registry entities for shared entities %u in scene %u", entity, sceneId);
        return {};
    }

    ComponentRecovery recovery;
    std::string recoveryDefKey = std::to_string(NULL_PROJECT_SCENE);
    recovery[recoveryDefKey].entity = registryEntity;
    recovery[recoveryDefKey].node = ProjectUtils::removeEntityComponent(group->registry.get(), registryEntity, componentType, group->registryEntities, encodeComponent);

    for (auto& [otherSceneId, sceneInstances] : group->instances) {
        SceneProject* otherScene = getScene(otherSceneId);
        if (otherScene){
            for (auto& instance : sceneInstances) {
                if ((otherSceneId != sceneId) || (instance.instanceId != instanceId) || removeToItself) {
                    Entity otherEntity = group->getLocalEntity(otherSceneId, instance.instanceId, registryEntity);

                    if (otherEntity != NULL_ENTITY) {
                        if (!group->hasComponentOverride(otherSceneId, otherEntity, componentType)) {
                            std::string recoveryKey = std::to_string(otherSceneId) + "_" + std::to_string(instance.instanceId);
                            recovery[recoveryKey].entity = otherEntity;
                            recovery[recoveryKey].node = ProjectUtils::removeEntityComponent(otherScene->scene, otherEntity, componentType, otherScene->entities, encodeComponent);

                            otherScene->isModified = true;
                        }
                    }
                }
            }
        }
    }

    group->isModified = true;

    return recovery;
}

void Editor::Project::saveSharedGroupToDisk(const std::filesystem::path& filepath) {
    SharedGroup* group = getSharedGroup(filepath);
    YAML::Node encodedNode = Stream::encodeEntity(NULL_ENTITY + 1, group->registry.get());
    if (group->isModified && encodedNode && !encodedNode.IsNull()) {
        std::filesystem::path fullSharedPath = getProjectPath() / filepath;
        std::ofstream fout(fullSharedPath.string());
        if (fout.is_open()) {  // Check if file opened successfully
            fout << YAML::Dump(encodedNode);
            fout.close();
            group->isModified = false;
        } else {
            Out::error("Failed to open file for writing: %s", fullSharedPath.string().c_str());
        }
    }
}

Editor::SharedGroup* Editor::Project::getSharedGroup(const std::filesystem::path& filepath){
    if (filepath.empty()){
        return nullptr;
    }
    auto it = sharedGroups.find(filepath);
    if (it != sharedGroups.end()) {
        return &it->second;
    }
    return nullptr;
}

const Editor::SharedGroup* Editor::Project::getSharedGroup(const std::filesystem::path& filepath) const {
    if (filepath.empty()){
        return nullptr;
    }
    auto it = sharedGroups.find(filepath);
    if (it != sharedGroups.end()) {
        return &it->second;
    }
    return nullptr;
}

std::filesystem::path Editor::Project::findGroupPathFor(uint32_t sceneId, Entity entity) const {
    for (const auto& [filepath, group] : sharedGroups){
        if (group.containsEntity(sceneId, entity)) {
            return filepath;
        }
    }
    return std::filesystem::path(); // empty path for none
}

bool Editor::Project::isEntityShared(uint32_t sceneId, Entity entity) const{
    for (const auto& [filepath, group] : sharedGroups){
        if (group.containsEntity(sceneId, entity)) {
            return true;
        }
    }
    return false;
}

std::vector<Editor::MergeResult> Editor::Project::mergeEntityNodes(const YAML::Node& extendNode, YAML::Node& outputNode) {
    std::vector<Editor::MergeResult> result;

    if (extendNode["entity"] && extendNode["entity"].IsScalar()) {
        outputNode["entity"] = extendNode["entity"];
    }

    uint64_t overrides = 0;

    if (extendNode["components"] && extendNode["components"].IsMap()) {
        for (auto it = extendNode["components"].begin(); it != extendNode["components"].end(); ++it) {
            std::string key = it->first.as<std::string>();
            outputNode["components"][key] = it->second;

            ComponentType compType = Catalog::getComponentType(key);
            uint64_t bit = 1ULL << static_cast<int>(compType);
            overrides |= bit;
        }
    }

    result.push_back({true, overrides});

    size_t extendChildrenSize = extendNode["children"]  ? extendNode["children"].size() : 0;

    for (size_t i = 0; i < extendChildrenSize; i++) {
        std::string extendType = extendNode["children"][i]["type"] ? extendNode["children"][i]["type"].as<std::string>() : "";

        if (extendType == "Entity"){

            YAML::Node newChild = YAML::Clone(extendNode["children"][i]);
            insertNewChild(outputNode, newChild, i);

            size_t entityCount = countEntitiesInBranch(newChild);
            for (size_t c = 0; c < entityCount; ++c) {
                result.push_back({false, 0});
            }

        }else if (extendType == "SharedEntityChild"){

            YAML::Node outputChild = outputNode["children"][i];
            YAML::Node extendChild = extendNode["children"][i];
            std::vector<Editor::MergeResult> newResults = mergeEntityNodes(extendChild, outputChild);
            std::copy(newResults.begin(), newResults.end(), std::back_inserter(result));

        }else if (extendType == "SharedEntity"){ // For nested shared entities

            YAML::Node newChild = YAML::Clone(extendNode["children"][i]);
            insertNewChild(outputNode, newChild, i);

        }

    }

    return result;
}

YAML::Node Editor::Project::clearEntitiesNode(YAML::Node node) {
    if (!node || !node.IsMap())
        return node;

    node.remove("entity");

    if (node["children"] && node["children"].IsSequence()) {
        for (size_t i = 0; i < node["children"].size(); ++i) {
            node["children"][i] = clearEntitiesNode(node["children"][i]);
        }
    }

    return node;
}

YAML::Node Editor::Project::changeEntitiesNode(Entity& firstEntity, YAML::Node node) {
    if (!node || !node.IsMap())
        return node;

    // Assign the current entity ID
    node["entity"] = firstEntity++;

    // Recursively process children
    if (node["children"] && node["children"].IsSequence()) {
        for (size_t i = 0; i < node["children"].size(); ++i) {
            node["children"][i] = changeEntitiesNode(firstEntity, node["children"][i]);
        }
    }

    return node;
}

void Editor::Project::collectEntities(const YAML::Node& entityNode, std::vector<Entity>& allEntities) {
    if (!entityNode || !entityNode.IsMap())
        return;

    if (entityNode["entity"]) {
        allEntities.push_back(entityNode["entity"].as<Entity>());
    }

    // Recursively process children
    if (entityNode["children"] && entityNode["children"].IsSequence()) {
        for (const auto& child : entityNode["children"]) {
            collectEntities(child, allEntities);
        }
    }
}

void Editor::Project::collectEntities(const YAML::Node& entityNode, std::vector<Entity>& allEntities, std::vector<Entity>& sharedEntities) {
    if (!entityNode || !entityNode.IsMap())
        return;

    if (entityNode["entity"]) {
        allEntities.push_back(entityNode["entity"].as<Entity>());
        if (entityNode["type"] && entityNode["type"].as<std::string>() != "Entity") {
            sharedEntities.push_back(entityNode["entity"].as<Entity>());
        }
    }

    // Recursively process children
    if (entityNode["children"] && entityNode["children"].IsSequence()) {
        for (const auto& child : entityNode["children"]) {
            collectEntities(child, allEntities, sharedEntities);
        }
    }
}

bool Editor::Project::sharedGroupPropertyChanged(uint32_t sceneId, Entity entity, ComponentType componentType, std::vector<std::string> properties, bool changeItself){
    fs::path filepath = findGroupPathFor(sceneId, entity);

    if (filepath.empty()) {
        Out::error("Entity %u in scene %u is not part of any shared group", entity, sceneId);
        return false;
    }

    SharedGroup* group = getSharedGroup(filepath);
    uint32_t instanceId = group->getInstanceId(sceneId, entity);

    if (!group->hasComponentOverride(sceneId, entity, componentType)){
        // Updating default entity
        Entity registryEntity = group->getRegistryEntity(sceneId, entity);
        if (registryEntity == NULL_ENTITY) {
            Out::error("Failed to find registry entity for shared entity %u in scene %u", entity, sceneId);
            return false;
        }
        EntityRegistry* registry = group->registry.get();
        if (properties.size() == 0){
            Catalog::copyComponent(getScene(sceneId)->scene, entity, registry, registryEntity, componentType);
        }else{
            for (const auto& property : properties) {
                Catalog::copyPropertyValue(getScene(sceneId)->scene, entity, registry, registryEntity, componentType, property);
            }
        }

        // Copy to corresponding entity in other scenes
        for (auto& [otherSceneId, sceneInstances] : group->instances) {
            for (auto& instance : sceneInstances) {
                if ((otherSceneId != sceneId) || (instance.instanceId != instanceId) || changeItself) {
                    Entity otherEntity = group->getLocalEntity(otherSceneId, instance.instanceId, registryEntity);

                    if (!group->hasComponentOverride(otherSceneId, otherEntity, componentType)) {
                        SceneProject* otherScene = getScene(otherSceneId);
                        if (otherScene) {
                            if (otherScene->isVisible){
                                otherScene->needUpdateRender = true;
                            }
                            if (properties.size() == 0){
                                Catalog::copyComponent(getScene(sceneId)->scene, entity, otherScene->scene, otherEntity, componentType);
                            }else{
                                for (const auto& property : properties) {
                                    Catalog::copyPropertyValue(getScene(sceneId)->scene, entity, otherScene->scene, otherEntity, componentType, property);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    group->isModified = true;

    return true;
}

bool Editor::Project::sharedGroupNameChanged(uint32_t sceneId, Entity entity, std::string name, bool changeItself){
    fs::path filepath = findGroupPathFor(sceneId, entity);

    if (filepath.empty()) {
        Out::error("Entity %u in scene %u is not part of any shared group", entity, sceneId);
        return false;
    }

    SharedGroup* group = getSharedGroup(filepath);
    uint32_t instanceId = group->getInstanceId(sceneId, entity);

    Entity registryEntity = group->getRegistryEntity(sceneId, entity);
    if (registryEntity == NULL_ENTITY) {
        Out::error("Failed to find registry entity for shared entity %u in scene %u", entity, sceneId);
        return false;
    }
    EntityRegistry* registry = group->registry.get();

    registry->setEntityName(registryEntity, name);

    for (auto& [otherSceneId, sceneInstances] : group->instances) {
        for (auto& instance : sceneInstances) {
            if ((otherSceneId != sceneId) || (instance.instanceId != instanceId) || changeItself) {
                Entity otherEntity = group->getLocalEntity(otherSceneId, instance.instanceId, registryEntity);

                SceneProject* otherScene = getScene(otherSceneId);
                if (otherScene) {

                    otherScene->scene->setEntityName(otherEntity, name);
                    otherScene->isModified = true;

                }

            }
        }
    }

    group->isModified = true;

    return true;
}

void Editor::Project::start(uint32_t sceneId) {
    SceneProject* sceneProject = getScene(sceneId);
    if (!sceneProject) {
        Out::error("Failed to find scene %u to start", sceneId);
        return;
    }

    updateAllScriptsProperties(sceneId);

    if (sceneProject->isModified && !sceneProject->filepath.empty()) {
        saveSceneToPath(sceneId, sceneProject->filepath);
    }
    Backend::getApp().getCodeEditor()->saveAll();

    // Save current scene state before starting
    sceneProject->playStateSnapshot = Stream::encodeSceneProject(this, sceneProject, true);

    sceneProject->playState = ScenePlayState::PLAYING;

    std::string libName = "projectlib";
    fs::path buildPath = getProjectInternalPath() / "build";

    std::vector<Editor::ScriptSource> scriptFiles = collectScriptSourceFiles();

    generator.build(getProjectPath(), getProjectInternalPath(), buildPath, libName, scriptFiles);

    std::thread connectThread([this, sceneProject, buildPath, libName]() {
        generator.waitForBuildToComplete();

        if (!generator.didLastBuildSucceed()) {
            sceneProject->playState = ScenePlayState::STOPPED;
            return;
        }

        if (conector.connect(buildPath, libName)) {
            conector.execute(sceneProject);
        } else {
            Out::error("Failed to connect to library");
            sceneProject->playState = ScenePlayState::STOPPED;
        }
    });
    connectThread.detach();

    pauseEngineScene(sceneProject, false);
    Engine::pauseGameEvents(false);
}

void Editor::Project::pause(uint32_t sceneId) {
    SceneProject* sceneProject = getScene(sceneId);
    if (!sceneProject) {
        Out::error("Failed to find scene %u to pause", sceneId);
        return;
    }

    if (sceneProject->playState == ScenePlayState::PLAYING) {
        sceneProject->playState = ScenePlayState::PAUSED;

        pauseEngineScene(sceneProject, true);
        Engine::pauseGameEvents(true);
    }
}

void Editor::Project::resume(uint32_t sceneId) {
    SceneProject* sceneProject = getScene(sceneId);
    if (!sceneProject) {
        Out::error("Failed to find scene %u to resume", sceneId);
        return;
    }

    if (sceneProject->playState == ScenePlayState::PAUSED) {
        sceneProject->playState = ScenePlayState::PLAYING;

        pauseEngineScene(sceneProject, false);
        Engine::pauseGameEvents(false);
    }
}

void Editor::Project::stop(uint32_t sceneId) {
    SceneProject* sceneProject = getScene(sceneId);
    if (!sceneProject) {
        Out::error("Failed to find scene %u to stop", sceneId);
        return;
    }

    sceneProject->playState = ScenePlayState::STOPPED;

    // Cleanup script instances before disconnecting
    if (conector.isLibraryConnected()) {
        conector.cleanup(sceneProject);
        conector.disconnect();
    }

    pauseEngineScene(sceneProject, true);
    Engine::pauseGameEvents(true);

    if (sceneProject->playStateSnapshot && !sceneProject->playStateSnapshot.IsNull()) {
        Stream::decodeScene(sceneProject->scene, sceneProject->playStateSnapshot["scene"]);

        auto entitiesNode = sceneProject->playStateSnapshot["entities"];
        for (const auto& entityNode : entitiesNode){
            Stream::decodeEntity(entityNode, sceneProject->scene, nullptr, this, sceneProject, NULL_ENTITY, true, false);
        }

        // Clear the snapshot
        sceneProject->playStateSnapshot = YAML::Node();
    }
}

void Editor::Project::debugSceneHierarchy(){
    if (SceneProject* sceneProject = getSelectedScene()){
        printf("Debug scene: %s\n", sceneProject->name.c_str());
        auto transforms = sceneProject->scene->getComponentArray<Transform>();
        for (int i = 0; i < transforms->size(); i++){
            auto transform = transforms->getComponentFromIndex(i);
            printf("Transform %i - Entity: %i - Parent: %i: %s\n", i, transforms->getEntity(i), transform.parent, sceneProject->scene->getEntityName(transforms->getEntity(i)).c_str());
        }
        printf("\n");
    }
}