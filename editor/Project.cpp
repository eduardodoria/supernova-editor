#include "Project.h"

#include "Backend.h"

#include "yaml-cpp/yaml.h"
#include <fstream>

#include "Out.h"
#include "subsystem/MeshSystem.h"
#include "command/CommandHandle.h"
#include "command/type/DeleteEntityCmd.h"
#include "Stream.h"
#include "Util.h"

using namespace Supernova;

Editor::Project::Project(){
    selectedScene = NULL_PROJECT_SCENE;

    tempPath = false;
    resourcesFocused = false;

    nextSceneId = 0;
}

bool Editor::Project::createNewProject(std::string projectName){
    try {
        projectPath = std::filesystem::temp_directory_path() / projectName;
        tempPath = true;

        if (!std::filesystem::exists(projectPath)) {
            std::filesystem::create_directory(projectPath);
            Out::info("Created project directory: %s", projectPath.string().c_str());
            createNewScene("New Scene");
        } else {
            Out::info("Project directory already exists: %s", projectPath.string().c_str());
            loadProject(projectPath);
        }

    } catch (const std::exception& e) {
        printf("Error: %s\n", e.what());
        return false;
    }

    return true;
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
        data.id = ++nextSceneId;
        data.name = "Unknown";
        data.scene = new Scene();
        data.sceneRender = new SceneRender(data.scene);
        data.selectedEntities.clear();
        data.needUpdateRender = true;
        data.isModified = false;
        data.filepath = filepath;

        Stream::decodeSceneProject(&data, sceneNode);

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

void Editor::Project::reset() {
    if (hasScenesUnsavedChanges()) {
        Backend::getApp().registerConfirmAlert(
            "Unsaved Changes",
            "There are unsaved changes. Do you want to save them before creating a new project?",
            [this]() {
                // Yes callback - save all and then reset
                saveAllScenes();
                performReset();
            },
            [this]() {
                // No callback - just reset without saving
                performReset();
            }
        );
    } else {
        // No unsaved changes, just reset
        performReset();
    }
}

void Editor::Project::performReset() {
    // Clear existing scenes
    for (auto& sceneProject : scenes) {
        deleteSceneProject(&sceneProject);
    }
    scenes.clear();
    Backend::getApp().resetLastActivatedScene();

    // Reset state
    selectedScene = NULL_PROJECT_SCENE;
    nextSceneId = 0;
    tempPath = true;
    //projectPath.clear();

    createNewScene("New Scene");
}

void Editor::Project::saveProject() {
    YAML::Node root = Stream::encodeProject(this);

    std::filesystem::path projectFile = projectPath / "project.yaml";
    std::ofstream fout(projectFile.string());
    fout << YAML::Dump(root);
    fout.close();
}

bool Editor::Project::loadProject(const std::filesystem::path& projectPath) {
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

void Editor::Project::saveScene(uint32_t sceneId) {
    SceneProject* sceneProject = getScene(sceneId);
    YAML::Node root = Stream::encodeSceneProject(sceneProject);

    if (sceneProject->filepath.empty()) {
        std::string suggestedName = sceneProject->name + ".scene";
        std::string savePath = Util::saveFileDialog(projectPath.string(), suggestedName, true);

        if (!savePath.empty()) {
            std::filesystem::path saveFilePath = std::filesystem::path(savePath);
            std::error_code ec;
            auto relPath = std::filesystem::relative(saveFilePath, projectPath, ec);

            if (ec || relPath.string().find("..") != std::string::npos) {
                Backend::getApp().registerAlert("Error", "Files must be saved within the project directory!");
                return;
            }

            sceneProject->filepath = savePath;
        } else {
            return;
        }
    }

    std::ofstream fout(sceneProject->filepath.string());
    fout << YAML::Dump(root);
    fout.close();

    sceneProject->isModified = false;

    saveProject();
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
    throw std::out_of_range("cannot find selected scene");
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

bool Editor::Project::isTempPath() const{
    return tempPath;
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