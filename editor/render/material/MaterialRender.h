#pragma once

#include "Scene.h"
#include "object/Camera.h"
#include "object/Object.h"
#include "object/Shape.h"
#include "object/Light.h"

namespace Supernova::Editor{

    class MaterialRender{
    private:
        Scene* scene;
        Camera* camera;

        Light* light;

        Shape* sphere;
    public:
        MaterialRender();
        virtual ~MaterialRender();

        void applyMaterial(const Material& material);
        const Material getMaterial();

        Framebuffer* getFramebuffer();
        Texture getTexture();
        Scene* getScene();
        Object* getObject();
    };

}