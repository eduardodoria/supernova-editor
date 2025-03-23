#ifndef RENDERUTIL_H
#define RENDERUTIL_H

namespace Supernova::Editor{

    enum class CursorSelected{
        POINTER,
        HAND
    };

    enum class GizmoSelected{
        TRANSLATE,
        ROTATE,
        SCALE,
        OBJECT2D
    };

    enum class GizmoSideSelected{
        NONE,
        X,
        Y,
        Z,
        XY,
        XZ,
        YZ,
        XYZ
    };

}

#endif /* RENDERUTIL_H */