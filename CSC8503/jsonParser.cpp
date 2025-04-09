#include "jsonParser.h"

#include <fstream>
#include <iostream>

using json = nlohmann::json;

bool jsonParser::LoadLevel(const std::string& filename, int& level, std::vector<InGameObject>& objects, std::vector<Enemy>& enemies) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open file " << filename << "\n";
        return false;
    }

    json jsonData;
    file >> jsonData; 

    level = jsonData["level"]; 

    objects.clear();
    enemies.clear();

    for (const auto& obj : jsonData["objects"]) {
        InGameObject gameObject;
        gameObject.id = obj["id"];
        gameObject.type = obj["type"];
        gameObject.position = { obj["position"]["x"], obj["position"]["y"], obj["position"]["z"] };
        gameObject.dimensions = { obj["dimensions"]["x"], obj["dimensions"]["y"], obj["dimensions"]["z"] };
        gameObject.orientation = { obj["orientation"]["x"], obj["orientation"]["y"], obj["orientation"]["z"] };

        objects.push_back(gameObject);
    }

    for (const auto& enemy : jsonData["enemies"]) {
        Enemy enemyData;
        Vector3 waypointPos;
        enemyData.name = enemy["name"];
        enemyData.position = { enemy["position"]["x"], enemy["position"]["y"], enemy["position"]["z"] };
        for (const auto& waypoint : enemy["waypoints"]) {
            waypointPos = { waypoint["x"], waypoint["y"], waypoint["z"] };
            enemyData.waypoints.push_back(waypointPos);
        }

        enemies.push_back(enemyData);
    }
    return true; 
}