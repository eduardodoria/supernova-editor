#pragma once

#include "util/ResourceProgress.h"

namespace Supernova::Editor {

    class LoadingWindow {
    private:
        bool wasShowing = false;

    public:
        LoadingWindow();
        ~LoadingWindow();

        void show();

    private:
        void drawProgressModal(const OverallBuildProgress& progress);
    };

}