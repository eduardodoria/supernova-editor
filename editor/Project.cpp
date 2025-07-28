#include "Project.h"

#include "Backend.h"

#include <fstream>

#include "render/SceneRender2D.h"
#include "render/SceneRender3D.h"

#include "AppSettings.h"
#include "Out.h"
#include "subsystem/MeshSystem.h"
#include "command/CommandHandle.h"
#include "command/type/DeleteEntityCmd.h"
#include "Stream.h"
#include "util/FileDialogs.h"
#include "util/SHA1.h"
#include "util/GraphicUtils.h"

#include "command/type/CreateEntityCmd.h"

using namespace Supernova;

Editor::EventBus Editor::Project::globalEventBus;

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

std::vector<size_t> Editor::Project::mergeEntityNodes(YAML::Node& loadedNode, const YAML::Node& extendNode, size_t& index){
    std::vector<size_t> extendedIndices;
    index++;

    size_t loadedChildrenSize = loadedNode["children"]  ? loadedNode["children"].size() : 0;
    size_t extendChildrenSize = extendNode["children"]  ? extendNode["children"].size() : 0;

    if (loadedChildrenSize < extendChildrenSize){
        for (size_t i = 0; i < extendChildrenSize; i++) {
            std::string extendType = extendNode["children"][i]["type"] ? extendNode["children"][i]["type"].as<std::string>() : "";

            if (extendType == "Entity"){
                YAML::Node newChild = YAML::Clone(extendNode["children"][i]);
                if (loadedNode["children"]){
                    // To insert at position i, we need to rebuild the sequence
                    YAML::Node tempChildren = YAML::Node(YAML::NodeType::Sequence);

                    // Copy elements before position i
                    for (size_t j = 0; j < i && j < loadedChildrenSize; j++) {
                        tempChildren.push_back(loadedNode["children"][j]);
                    }
                    // Insert the new child
                    tempChildren.push_back(newChild);
                    // Copy remaining elements
                    for (size_t j = i; j < loadedChildrenSize; j++) {
                        tempChildren.push_back(loadedNode["children"][j]);
                    }

                    loadedChildrenSize++;
                    loadedNode["children"] = tempChildren;
                }else{
                    loadedNode["children"] = YAML::Node(YAML::NodeType::Sequence);
                    loadedNode["children"].push_back(newChild);
                }
                extendedIndices.push_back(index);
            }

            YAML::Node loadedChild = loadedNode["children"][i];
            YAML::Node extendChild = extendNode["children"][i];
            std::vector<size_t> newIndices = mergeEntityNodes(loadedChild, extendChild, index);
            std::copy(newIndices.begin(), newIndices.end(), std::back_inserter(extendedIndices));
        }
    }else{
        for (size_t i = 0; i < loadedChildrenSize; i++) {
            YAML::Node loadedChild = loadedNode["children"][i];
            YAML::Node extendChild = YAML::Node();
            if (extendChildrenSize > 0){
                size_t extendedIndex = i < extendChildrenSize ? i : extendChildrenSize - 1;
                extendChild = extendNode["children"][extendedIndex];
            }
            std::vector<size_t> newIndices = mergeEntityNodes(loadedChild, extendChild, index);
            std::copy(newIndices.begin(), newIndices.end(), std::back_inserter(extendedIndices));
        }
    }

    return extendedIndices;
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
    bool hasSharedEntities = false;
    for (const auto& [filepath, group] : sharedGroups) {
        if (group.members.find(sceneId) != group.members.end()) {
            hasSharedEntities = true;
            break;
        }
    }

    if (hasSharedEntities) {
        saveSharedGroupsToDisk();
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

void Editor::Project::setupSharedGroupEventSubscriptions(const std::filesystem::path& filepath) {
    // Subscribe to component changes for this entity
    getEventBus().subscribe(EventType::ComponentChanged,
        [this, filepath](const Event& e) {
            // Check if this event is for any entity in our shared group
            const SharedGroup* group = getSharedGroup(filepath);
            if (group) {
                if (group->containsEntity(e.sceneId, e.entity)){
                    saveSharedGroup(filepath, e.sceneId);
                }

                // Copy changes to corresponding entities in other scenes
                const std::vector<Entity>& sourceEntities = group->getAllEntities(e.sceneId);
                auto sourceIt = std::find(sourceEntities.begin(), sourceEntities.end(), e.entity);
                if (sourceIt != sourceEntities.end()) {
                    size_t entityIndex = std::distance(sourceEntities.begin(), sourceIt);

                    // Copy to corresponding entity in other scenes
                    for (const auto& [otherSceneId, otherEntities] : group->members) {
                        if (otherSceneId != e.sceneId && entityIndex < otherEntities.size()) {
                            Entity otherEntity = otherEntities[entityIndex];
                            SceneProject* otherScene = getScene(otherSceneId);
                            if (otherScene) {
                                otherScene->needUpdateRender = true;
                                for (const auto& property : e.properties) {
                                    Catalog::copyPropertyValue(getScene(e.sceneId)->scene, e.entity, otherScene->scene, otherEntity, e.compType, property);
                                }
                            }
                        }
                    }
                }
            }
        });

    // Also subscribe to component additions and removals
    getEventBus().subscribe(EventType::ComponentAdded,
        [this, filepath](const Event& e) {
            const SharedGroup* group = getSharedGroup(filepath);
            if (group && group->containsEntity(e.sceneId, e.entity)){
                saveSharedGroup(filepath, e.sceneId);
            }
        });

    getEventBus().subscribe(EventType::ComponentRemoved,
        [this, filepath](const Event& e) {
            const SharedGroup* group = getSharedGroup(filepath);
            if (group && group->containsEntity(e.sceneId, e.entity)){
                saveSharedGroup(filepath, e.sceneId);
            }
        });
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

fs::path Editor::Project::getThumbsDir() const{
    return getProjectPath() / ".supernova" / "thumbs";
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

Editor::EventBus& Editor::Project::getEventBus(){
    return globalEventBus;
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
    branchEntities.push_back(entity); // Root entity first

    // Add all children if the entity has Transform component
    Scene* scene = sceneProject->scene;
    if (scene->getSignature(entity).test(scene->getComponentId<Transform>())) {
        auto transforms = scene->getComponentArray<Transform>();
        size_t firstIndex = transforms->getIndex(entity);
        size_t branchIndex = scene->findBranchLastIndex(entity);

        for (size_t i = firstIndex + 1; i <= branchIndex; i++) {
            Entity childEntity = transforms->getEntity(i);
            // Only add if it's part of this scene's entities
            if (std::find(sceneProject->entities.begin(), sceneProject->entities.end(), childEntity) != sceneProject->entities.end()) {
                branchEntities.push_back(childEntity);
            }
        }
    }

    // Create new group
    SharedGroup group;
    group.members[sceneId] = branchEntities;
    group.cachedYaml = std::make_shared<YAML::Node>(std::move(entityNode));
    group.isModified = false;
    sharedGroups.emplace(filepath, std::move(group));

    // Set up event subscriptions for this shared group
    setupSharedGroupEventSubscriptions(filepath);

    sceneProject->isModified = true;
    return true;
}

bool Editor::Project::importSharedEntity(SceneProject* sceneProject, const std::filesystem::path& filepath, bool needSaveScene, YAML::Node extendNode) {
    if (!filepath.is_relative()) {
        Out::error("Shared entity filepath must be relative: %s", filepath.string().c_str());
        return false;
    }

    auto it = sharedGroups.find(filepath);
    bool isNewGroup = false;

    if (it == sharedGroups.end()) {
        // Entity doesn't exist in any scene yet - create new SharedGroup
        SharedGroup newGroup;
        newGroup.isModified = false;

        auto [newIt, inserted] = sharedGroups.emplace(filepath, std::move(newGroup));
        it = newIt;
        isNewGroup = true;
    } else {
        // do not re‐import if already present
        if (it->second.members.count(sceneProject->id)) return false;
    }

    auto& group = it->second;

    std::shared_ptr<YAML::Node> node;
    // Use cached YAML if available and modified, otherwise load from file
    if (group.isModified && group.cachedYaml && !group.cachedYaml->IsNull()) {
        node = group.cachedYaml;
    } else {
        try {
            std::filesystem::path fullSharedPath = getProjectPath() / filepath;
            YAML::Node loadedNode = YAML::LoadFile(fullSharedPath.string());
            node = std::make_shared<YAML::Node>(std::move(loadedNode));
            group.cachedYaml = node; // cache it
        } catch (const YAML::Exception& e) {
            Out::error("Failed to load shared entity file: %s", e.what());
            return false;
        } catch (const std::exception& e) {
            Out::error("Failed to load shared entity file: %s", e.what());
            return false;
        }
    }

    std::vector<size_t> indicesOfExtend;

    // Merge extendNode with loadedNode if extendNode is provided
    if (extendNode && !extendNode.IsNull()) {
        // Create a copy of the loaded node for merging
        YAML::Node mergedNode = YAML::Clone(*node);
        size_t globalIndex = 0;
        indicesOfExtend = mergeEntityNodes(mergedNode, extendNode, globalIndex);
        node = std::make_shared<YAML::Node>(std::move(mergedNode));
    }

    // decode into brand‐new local entities (root + children)
    Scene* scene = sceneProject->scene;
    std::vector<Entity> newEntities = Stream::decodeEntity(scene, *node);
    std::copy(newEntities.begin(), newEntities.end(), std::back_inserter(sceneProject->entities));

    // Remove from newEntities all elements whose index is in indicesOfExtend
    if (!indicesOfExtend.empty()) {
        // Sort indices in descending order to avoid invalidating indices when erasing
        std::sort(indicesOfExtend.rbegin(), indicesOfExtend.rend());
        for (size_t idx : indicesOfExtend) {
            if (idx < newEntities.size()) {
                newEntities.erase(newEntities.begin() + idx);
            }
        }
    }

    // Store all entities in the shared group
    group.members[sceneProject->id] = newEntities;

    sceneProject->isModified = needSaveScene;

    // Set up event subscriptions for new shared groups
    if (isNewGroup) {
        setupSharedGroupEventSubscriptions(filepath);
    }

    return true;
}

void Editor::Project::saveSharedGroup(const std::filesystem::path& filepath, uint32_t sceneId){
    auto it = sharedGroups.find(filepath);
    if (it == sharedGroups.end()) return;

    auto& group = it->second;

    // Check if the provided sceneId is part of this shared group
    auto memberIt = group.members.find(sceneId);
    if (memberIt == group.members.end() || memberIt->second.empty()) {
        Out::error("Scene ID %u is not part of shared group at %s", sceneId, filepath.string().c_str());
        return;
    }

    // Use the root entity from the provided scene as authoritative source
    Entity rootEntity = memberIt->second[0]; // First entity is always the root
    Scene* scene = getScene(sceneId)->scene;

    // Re‐serialize the entire branch to memory cache
    YAML::Node encodedNode = Stream::encodeEntityBranch(rootEntity, this, getScene(sceneId));
    group.cachedYaml = std::make_shared<YAML::Node>(std::move(encodedNode));
    group.isModified = true;
}

void Editor::Project::saveSharedGroupsToDisk(){
    for (auto& [filepath, group] : sharedGroups) {
        // Add proper null checks
        if (group.isModified && group.cachedYaml && !group.cachedYaml->IsNull()) {
            std::filesystem::path fullSharedPath = getProjectPath() / filepath;
            std::ofstream fout(fullSharedPath.string());
            if (fout.is_open()) {  // Check if file opened successfully
                fout << YAML::Dump(*group.cachedYaml);
                fout.close();
                group.isModified = false;
            } else {
                Out::error("Failed to open file for writing: %s", fullSharedPath.string().c_str());
            }
        }
    }
}

Editor::SharedGroup* Editor::Project::getSharedGroup(const std::filesystem::path& filepath){
    auto it = sharedGroups.find(filepath);
    if (it != sharedGroups.end()) {
        return &it->second;
    }
    return nullptr;
}

const Editor::SharedGroup* Editor::Project::getSharedGroup(const std::filesystem::path& filepath) const {
    auto it = sharedGroups.find(filepath);
    if (it != sharedGroups.end()) {
        return &it->second;
    }
    return nullptr;
}

std::filesystem::path Editor::Project::findGroupPathFor(uint32_t sceneId, Entity e) const {
    for (const auto& [filepath, group] : sharedGroups){
        if (group.containsEntity(sceneId, e)) {
            return filepath;
        }
    }
    return std::filesystem::path(); // empty path for none
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