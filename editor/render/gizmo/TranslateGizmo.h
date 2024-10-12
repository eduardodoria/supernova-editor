#ifndef TRANSLATEGIZMO_H
#define TRANSLATEGIZMO_H

#include "object/Camera.h"
#include "object/Object.h"
#include "object/Shape.h"
#include "object/ui/Image.h"

#include "render/RenderUtil.h"

namespace Supernova::Editor{

    class TranslateGizmo: public Object{
    private:
        Shape* sphere;
        Shape* xaxis;
        Shape* yaxis;
        Shape* zaxis;
        Shape* xarrow;
        Shape* yarrow;
        Shape* zarrow;

        static const Vector4 centerColor;
        static const Vector4 xaxisColor;
        static const Vector4 yaxisColor;
        static const Vector4 zaxisColor;
        static const Vector4 centerColorHightlight;
        static const Vector4 xaxisColorHightlight;
        static const Vector4 yaxisColorHightlight;
        static const Vector4 zaxisColorHightlight;

    public:
        TranslateGizmo(Scene* scene);

        GizmoSideSelected checkHoverHighlight(Ray& ray);
    };

}

#endif /* TRANSLATEGIZMO_H */