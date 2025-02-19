#ifndef ROTATEGIZMO_H
#define ROTATEGIZMO_H

#include "object/Camera.h"
#include "object/Object.h"
#include "object/Shape.h"
#include "object/ui/Image.h"
#include "object/Lines.h"

#include "render/RenderUtil.h"

namespace Supernova::Editor{

    class RotateGizmo: public Object{
    private:
        Shape* maincircle;
        Shape* xcircle;
        Shape* ycircle;
        Shape* zcircle;

        Lines* lines;

        std::vector<AABB> xcircleAABBs;
        std::vector<AABB> ycircleAABBs;
        std::vector<AABB> zcircleAABBs;

        static const Vector3 mainColor;
        static const Vector3 xaxisColor;
        static const Vector3 yaxisColor;
        static const Vector3 zaxisColor;
        static const Vector3 lineColor;
        static const Vector3 mainColorHightlight;
        static const Vector3 xaxisColorHightlight;
        static const Vector3 yaxisColorHightlight;
        static const Vector3 zaxisColorHightlight;
        static const float circleAlpha;

        std::vector<AABB> createHalfTorus(Entity entity, float radius, float ringRadius, unsigned int sides, unsigned int rings);

    public:
        RotateGizmo(Scene* scene);
        virtual ~RotateGizmo();

        void updateRotations(Camera* camera);
        void drawLine(Vector3 point);
        void removeLine();

        GizmoSideSelected checkHoverHighlight(Ray& ray);
    };

}

#endif /* ROTATEGIZMO_H */