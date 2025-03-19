#pragma once


#include "object/Object.h"
#include "object/ui/Polygon.h"
#include "object/Lines.h"

#include "render/RenderUtil.h"

namespace Supernova::Editor{

    class Object2DGizmo: public Object{
    private:
        Polygon* polygon;

    public:
        Object2DGizmo(Scene* scene);
        virtual ~Object2DGizmo();

    };

}