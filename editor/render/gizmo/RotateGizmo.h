#ifndef ROTATEGIZMO_H
#define ROTATEGIZMO_H

#include "object/Camera.h"
#include "object/Object.h"
#include "object/Shape.h"
#include "object/ui/Image.h"

#include "render/RenderUtil.h"

namespace Supernova::Editor{

    class RotateGizmo: public Object{
    private:
        Shape* maincircle;
        Shape* xcircle;
        Shape* ycircle;
        Shape* zcircle;

        static const Vector3 mainColor;
        static const Vector3 xaxisColor;
        static const Vector3 yaxisColor;
        static const Vector3 zaxisColor;
        static const Vector3 centerColorHightlight;
        static const Vector3 xaxisColorHightlight;
        static const Vector3 yaxisColorHightlight;
        static const Vector3 zaxisColorHightlight;
        static const float circleAlpha;

    public:
        RotateGizmo(Scene* scene);

        GizmoSideSelected checkHoverHighlight(Ray& ray);
    };

}

#endif /* ROTATEGIZMO_H */