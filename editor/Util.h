#ifndef EDITORUTIL_H
#define EDITORUTIL_H

#include <vector>
#include <string>
#include "Backend.h"
#include "imgui.h"
#include "nfd.hpp"

namespace Supernova::Editor{
    class Util{

    public:
        inline static std::vector<std::string> getStringsFromPayload(const ImGuiPayload* payload){
            const char* data = static_cast<const char*>(payload->Data);
            size_t dataSize = payload->DataSize;

            std::vector<std::string> receivedStrings;
            size_t offset = 0;

            while (offset < dataSize) {
                // Read null-terminated strings from the payload
                const char* str = &data[offset];
                receivedStrings.push_back(std::string(str));
                offset += strlen(str) + 1; // Move past the string and null terminator
            }

            return receivedStrings;
        }

        inline static std::string openFileDialog(std::string defaultPath = "", bool onlyImages = false){
            std::string retPath;
            char* path;
            nfdopendialogu8args_t args = {0};

            if (onlyImages){
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
    };

}

#endif /* EDITORUTIL_H */