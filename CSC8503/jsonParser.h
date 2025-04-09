#ifndef LEVEL_LOADER_H
#define LEVEL_LOADER_H

#include <vector>
#include <string>
#include <fstream>
#include "../CSC8503/json.hpp"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"
#include "Assets.h"

using json = nlohmann::json;

struct InGameObject {
    int id;
    std::string type;
    Vector3 position;
    Vector3 dimensions;
    Vector3 orientation;
};

struct Enemy {
    std::string name;
    Vector3 position;
    std::vector<Vector3> waypoints;
};

struct LevelData {
    int level;
    std::vector<InGameObject> objects;
    std::vector<Enemy> enemies;
};


class jsonParser {
public:
    static bool LoadLevel(const std::string& filename, int& level, std::vector<InGameObject>& objects, std::vector<Enemy>& enemies);
};

#endif 