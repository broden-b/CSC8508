using System.Collections.Generic;
using System.IO;
using UnityEngine;

[System.Serializable]
public class Position
{
    public float x, y, z;
}

[System.Serializable]
public class Dimensions
{
    public float x, y, z;
}

[System.Serializable]
public class Orientation
{
    public float x, y, z;
}

[System.Serializable]
public class GameObjectData
{
    public int id;
    public string type;
    public Position position;
    public Dimensions dimensions;
    public Orientation orientation;
}

[System.Serializable]
public class EnemyData
{
    public string name;
    public Position position;
    public List<Position> waypoints = new List<Position>();
}

[System.Serializable]
public class LevelData
{
    public int level = 1; // changable level number
    public List<GameObjectData> objects = new List<GameObjectData>();
    public List<EnemyData> enemies = new List<EnemyData>();
}

public class convertToJson : MonoBehaviour
{
    public string fileName = "level.json";

    void Start()
    {
        ExportLevel();
    }

    void ExportLevel()
    {
        LevelData levelData = new LevelData();
        GameObject[] allObjects = FindObjectsOfType<GameObject>();

        int idCounter = 1;
        foreach (GameObject obj in allObjects) {
            if (!obj.activeInHierarchy) continue;

            if (obj.CompareTag("Enemy")) {
                EnemyData enemyData = new EnemyData {
                    name = obj.name,
                    position = new Position {
                        x = obj.transform.position.x,
                        y = obj.transform.position.y,
                        z = obj.transform.position.z
                    }
                };

                foreach (Transform child in obj.transform) {
                    if (child.CompareTag("Waypoint")) {
                        Vector3 worldPosition = child.TransformPoint(Vector3.zero);

                        Position waypointPos = new Position {
                            x = worldPosition.x,
                            y = worldPosition.y,
                            z = worldPosition.z,
                        };

                        enemyData.waypoints.Add(waypointPos);
                    }
                }
                
                levelData.enemies.Add(enemyData);
            }
            else if (!obj.CompareTag("Waypoint")) {
                GameObjectData gameObjectData = new GameObjectData {
                    id = idCounter++,
                    type = obj.tag != "Untagged" ? obj.tag : obj.name, 

                    position = new Position {
                        x = obj.transform.position.x,
                        y = obj.transform.position.y,
                        z = obj.transform.position.z
                    },
                    dimensions = new Dimensions {
                        x = obj.transform.localScale.x,
                        y = obj.transform.localScale.y,
                        z = obj.transform.localScale.z
                    },
                    orientation = new Orientation {
                        x = obj.transform.eulerAngles.x, 
                        y = obj.transform.eulerAngles.y,
                        z = obj.transform.eulerAngles.z
                    }
                };
                levelData.objects.Add(gameObjectData); 
            }
        }
        string json = JsonUtility.ToJson(levelData, true); 
        File.WriteAllText(Application.persistentDataPath + "/" + fileName, json); 
        Debug.Log("Level exported to " + Application.persistentDataPath + "/" + fileName);
    }
}
