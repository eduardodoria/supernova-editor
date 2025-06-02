#include "ResourceProgress.h"
#include <algorithm>

using namespace Supernova::Editor;

std::mutex ResourceProgressTracker::progressMutex;
std::unordered_map<uint64_t, ResourceBuildInfo> ResourceProgressTracker::activeBuilds;
uint64_t ResourceProgressTracker::mostRecentBuildId = 0;

void ResourceProgressTracker::startBuild(uint64_t id, ResourceType type, const std::string& name) {
    std::lock_guard<std::mutex> lock(progressMutex);

    ResourceBuildInfo info;
    info.type = type;
    info.name = name;
    info.progress = 0.0f;
    info.isActive = true;
    info.startTime = std::chrono::steady_clock::now();

    activeBuilds[id] = info;
    mostRecentBuildId = id;
}

void ResourceProgressTracker::updateProgress(uint64_t id, float progress) {
    std::lock_guard<std::mutex> lock(progressMutex);

    auto it = activeBuilds.find(id);
    if (it != activeBuilds.end()) {
        it->second.progress = std::clamp(progress, 0.0f, 1.0f);
    }
}

void ResourceProgressTracker::completeBuild(uint64_t id) {
    std::lock_guard<std::mutex> lock(progressMutex);
    activeBuilds.erase(id);
}

void ResourceProgressTracker::failBuild(uint64_t id) {
    std::lock_guard<std::mutex> lock(progressMutex);
    activeBuilds.erase(id);
}

bool ResourceProgressTracker::hasActiveBuilds() {
    std::lock_guard<std::mutex> lock(progressMutex);
    return !activeBuilds.empty();
}

OverallBuildProgress ResourceProgressTracker::getOverallProgress() {
    std::lock_guard<std::mutex> lock(progressMutex);

    OverallBuildProgress overall;

    if (activeBuilds.empty()) {
        return overall; // All defaults to false/0
    }

    overall.hasActiveBuilds = true;
    overall.totalBuilds = static_cast<int>(activeBuilds.size());

    // Calculate total progress across all builds
    float totalProgress = 0.0f;
    for (const auto& [id, build] : activeBuilds) {
        totalProgress += build.progress;
    }
    overall.totalProgress = totalProgress / overall.totalBuilds;

    // Get information about the most recent build
    auto recentIt = activeBuilds.find(mostRecentBuildId);
    if (recentIt != activeBuilds.end()) {
        overall.currentBuildName = recentIt->second.name;
        overall.currentBuildType = recentIt->second.type;
    } else if (!activeBuilds.empty()) {
        // Fallback if most recent isn't found
        overall.currentBuildName = activeBuilds.begin()->second.name;
        overall.currentBuildType = activeBuilds.begin()->second.type;
    }

    return overall;
}

ResourceBuildInfo ResourceProgressTracker::getCurrentBuild() {
    std::lock_guard<std::mutex> lock(progressMutex);

    if (activeBuilds.empty()) {
        return {};
    }

    auto it = activeBuilds.find(mostRecentBuildId);
    if (it != activeBuilds.end()) {
        return it->second;
    }

    return activeBuilds.begin()->second;
}

int ResourceProgressTracker::getActiveBuildCount() {
    std::lock_guard<std::mutex> lock(progressMutex);
    return static_cast<int>(activeBuilds.size());
}

std::vector<ResourceBuildInfo> ResourceProgressTracker::getAllActiveBuilds() {
    std::lock_guard<std::mutex> lock(progressMutex);

    std::vector<ResourceBuildInfo> builds;
    builds.reserve(activeBuilds.size());

    for (const auto& [id, build] : activeBuilds) {
        builds.push_back(build);
    }

    // Sort by start time (most recent first) or by name for consistent ordering
    std::sort(builds.begin(), builds.end(), 
        [](const ResourceBuildInfo& a, const ResourceBuildInfo& b) {
            return a.startTime > b.startTime; // Most recent first
        });

    return builds;
}

std::string ResourceProgressTracker::getResourceTypeName(ResourceType type) {
    switch (type) {
        case ResourceType::Shader: return "Shader";
        case ResourceType::Texture: return "Texture";
        case ResourceType::Model: return "Model";
        case ResourceType::Audio: return "Audio";
        case ResourceType::Material: return "Material";
        default: return "Resource";
    }
}