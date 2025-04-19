#pragma once

#include <vector>
#include <string>
#include "Backend.h"
#include "imgui.h"
#include "nfd.hpp"

namespace Supernova::Editor{
    class FileDialogs{

    public:
        inline static std::string openFileDialog(std::string defaultPath = "", bool onlyImages = false, bool selectDirectory = false) {
            std::string retPath;

            if (selectDirectory) {
                // Directory selection
                nfdu8char_t* outPath;
                nfdpickfolderu8args_t args = {0};
                args.defaultPath = defaultPath.c_str();
                args.parentWindow = *static_cast<nfdwindowhandle_t*>(Backend::getNFDWindowHandle());

                // Use NFD_PickFolderU8_With for consistency with other functions
                nfdresult_t result = NFD_PickFolderU8_With(&outPath, &args);

                if (result == NFD_OKAY) {
                    retPath = outPath;
                    NFD_FreePathU8(outPath);
                } else if (result == NFD_ERROR) {
                    printf("Error: %s\n", NFD_GetError());
                }
            } else {
                // File selection
                nfdu8char_t* path;
                nfdopendialogu8args_t args = {0};

                if (onlyImages) {
                    nfdfilteritem_t filterItem[1] = {
                        { "Image files", "jpeg,jpg,png,bmp,psd,tga,gif,hdr,pic,pnm" }
                    };
                    args.filterCount = 1;
                    args.filterList = filterItem;
                }
                args.defaultPath = defaultPath.c_str();
                args.parentWindow = *static_cast<nfdwindowhandle_t*>(Backend::getNFDWindowHandle());

                const nfdresult_t res = NFD_OpenDialogU8_With(&path, &args);
                switch (res) {
                    case NFD_OKAY:
                        retPath = path;
                        NFD_FreePathU8(path);
                        break;
                    case NFD_ERROR:
                        printf("Error: %s", NFD_GetError());
                        break;
                    default:
                        break;
                }
            }

            return retPath;
        }

        inline static std::vector<std::string> openFileDialogMultiple(){
            std::vector<std::string> filePaths;
            const nfdpathset_t* pathSet;
            nfdopendialogu8args_t args = {0};

            args.parentWindow = *static_cast<nfdwindowhandle_t*>(Backend::getNFDWindowHandle());

            const nfdresult_t res = NFD_OpenDialogMultipleU8_With(&pathSet, &args);
            switch (res) {
                case NFD_OKAY:
                    nfdpathsetsize_t num_paths;
                    if (NFD_PathSet_GetCount(pathSet, &num_paths) != NFD_OKAY) {
                        printf("Error: NFD_PathSet_GetCount failed: %s\n", NFD_GetError());
                        break;
                    }
                    nfdpathsetsize_t i;
                    for (i = 0; i != num_paths; ++i) {
                        char* path;
                        if (NFD_PathSet_GetPathU8(pathSet, i, &path) != NFD_OKAY) {
                            printf("Error: NFD_PathSet_GetPathU8 failed: %s\n", NFD_GetError());
                            break;
                        }
                        filePaths.push_back(path);
                        NFD_PathSet_FreePathU8(path);
                    }
                    NFD_PathSet_Free(pathSet);

                    break;
                case NFD_ERROR:
                    printf("Error: %s", NFD_GetError());
                    break;
                default:
                    break;
            }

            return filePaths;
        }

        inline static std::string saveFileDialog(std::string defaultPath = "", std::string defaultName = "", bool sceneOnly = false) {
            std::string retPath;
            char* path;
            nfdsavedialogu8args_t args = {0};

            if (sceneOnly) {
                nfdfilteritem_t filterItem[1] = {
                    { "Scene files", "scene" }
                };
                args.filterCount = 1;
                args.filterList = filterItem;
            }
            args.defaultPath = defaultPath.c_str();
            args.defaultName = defaultName.c_str();  // Set the default filename
            args.parentWindow = *static_cast<nfdwindowhandle_t*>(Backend::getNFDWindowHandle());

            const nfdresult_t res = NFD_SaveDialogU8_With(&path, &args);
            switch (res) {
                case NFD_OKAY:
                    retPath = path;
                    NFD_FreePathU8(path);
                    break;
                case NFD_ERROR:
                    printf("Error: %s", NFD_GetError());
                    break;
                default:
                    break;
            }

            return retPath;
        }
    };

}