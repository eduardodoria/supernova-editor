#pragma once


#include "object/Object.h"
#include "object/ui/Polygon.h"
#include "object/Lines.h"

#include "render/RenderUtil.h"

namespace Supernova::Editor{

    class Object2DGizmo: public Object{
    private:
        Polygon* rects[8];

        float width;
        float height;

        static const float rectSize;
        static const float sizeOffset;

        void updateRects();

    public:
        Object2DGizmo(Scene* scene);
        virtual ~Object2DGizmo();

        void setSize(float width, float height);

        Gizmo2DSideSelected checkHover(const Ray& ray, const AABB& aabb);
    };

}