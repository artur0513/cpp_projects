#pragma once
#include "Texture.h"
#include "Resources.h"

namespace ogl {

    struct Material {
        std::string name;

        Texture* diffuseTexture = nullptr;
        Texture* normalTexture = nullptr;
        //add more then needed
    };

    namespace Resources {
        Material getMaterial(std::string materialName);
        bool loadMaterials(std::string filename);
    }

}
