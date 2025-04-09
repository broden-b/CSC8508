#pragma once

#include "PhysicsSystem.h"
#include "../CSC8503/jsonParser.h"
#include "../CSC8503/PrisonEscape/Scripts/puzzle/Button.h"
#include "../CSC8503/PrisonEscape/Scripts/puzzle/puzzleT.h"
#include "../CSC8503/PrisonEscape/Levels/Level.h"
#include "../CSC8503/PrisonEscape/Scripts/Player/Player.h"
#include "../CSC8503/PrisonEscape/Scripts/PatrolEnemy/PatrolEnemy.h"
#include "../CSC8503/PrisonEscape/Scripts/CameraEnemy/CameraEnemy.h"




namespace NCL {
	namespace CSC8503 {
		constexpr float PLAYER_MESH_SIZE = 3.0f;
		constexpr float PLAYER_INVERSE_MASS = 0.5f;
		constexpr float PATROL_ENEMY_MESH_SIZE = 3.0f;
		constexpr float PATROL_ENEMY_INVERSE_MASS = 0.1f;
		constexpr float PURSUIT_ENEMY_MESH_SIZE = 3.0f;
		constexpr float PURSUIT_ENEMY_INVERSE_MASS = 0.1f;
		constexpr float CAMERA_ENEMY_MESH_SIZE = 3.0f;
		constexpr float CAMERA_ENEMY_INVERSE_MASS = 0.1f;

		class GameWorld;
		class Level;
		class Button;
		class Player;
		class PatrolEnemy;
		class PursuitEnemy;
		class jsonParser;
		class GameTechRenderer;
		class AnimationController;
		class Exit;

		class GameLevelManager {
		public:
			GameLevelManager(GameWorld* existingWorld, GameTechRenderer* existingRenderer, string LevelToLoad, bool multiplayerStatus, bool isServer);
			~GameLevelManager();
			virtual void UpdateGame(float dt);

			void InitAssets();
			void InitAnimationObjects() const;

			Player* AddPlayerToWorld(const Transform& transform, const std::string& playerName);
			void AddComponentsToPlayer(Player& playerObj, const Transform& transform);

			Vector3 GetP1Position() { return playerOne->GetTransform().GetPosition(); }
			Vector3 GetP2Position() { return playerTwo->GetTransform().GetPosition(); }

			PatrolEnemy* AddPatrolEnemyToWorld(const std::string& patrolEnemyName, const std::vector<Vector3>& patrolPoints, const Vector3& spawnPoint, Player* player);
			void AddComponentsToPatrolEnemy(PatrolEnemy& enemyObj, const Transform& transform);

			PursuitEnemy* AddPursuitEnemyToWorld(const std::string& pursuitEnemyName, const Vector3& position, const std::vector<Vector3>& patrolPoints, Player* player);
			void AddComponentsToPursuitEnemy(PursuitEnemy& enemyObj, const Transform& transform);

			CameraEnemy* AddCameraEnemyToWorld(const std::string& cameraEnemyName, const Vector3& spawnPoint, Player* player);
			void AddComponentsToCameraEnemy(CameraEnemy& enemyObj, const Transform& transform);

			void loadMap(std::string levelToLoad);

		public:
			Level* GetCurrentLevel() { return mCurrentLevel; }
			void SetCurrentLevel(Level* level) { mCurrentLevel = level; }
			void AddLevel(Level* newLevel) { mLevelStack.push(newLevel); }
			void ClearLevel();
			AnimationController* GetAnimator() { return mAnimator; }

			static GameLevelManager* GetGameLevelManager() { return manager; }

			Player* GetPlayerOne() { return playerOne; }
			Player* GetPlayerTwo() { return playerTwo; }
			void SetLevelToLoad(string levelName) { mLevelToLoad = levelName; }

			std::vector<string> LoadingScreenText = { "One Kiran Added", "Diddy Spawned", "Soaps Added", "Supreme","One PS5 Renderer Added", "I ran out of ideas", "40k debt" };

		private:
			GameWorld* mWorld;
			static GameLevelManager* manager;
			GameTechRenderer* mRenderer;
			PhysicsSystem* mPhysics;
			Level* mCurrentLevel;
			Vector3 P1Position;
			Vector3 P2Position;

			Player* playerOne = nullptr;
			Player* playerTwo = nullptr;

			string mLevelToLoad = "Level1";

			// for handling multiple buttons/boxes in a level
			int boxNumber;
			std::vector<GameObject*> boxes;
			std::vector<Button*> buttons;
			std::vector<Door*> buttonDoors;
			std::vector<PressableDoor*>pressDoors;
			std::vector<ButtonTrigger*> buttonss;
			std::stack<Level*> mLevelStack;

			AnimationController* mAnimator;

			vector<GameObject*> mUpdatableObjectList;

			std::unordered_map<std::string, Mesh*> mMeshList;
			std::unordered_map<std::string, Shader*> mShaderList;
			std::unordered_map<std::string, Texture*> mTextureList;
			std::unordered_map<std::string, MeshAnimation*> mAnimationList;
			std::unordered_map<std::string, MeshMaterial*> mMaterialList;
			std::unordered_map<std::string, vector<int>> mMeshMaterialsList;
			std::unordered_map<std::string, std::string> mLevelList;
			std::map<std::string, MeshAnimation*> mPreLoadedAnimationList;

			GameObject* AddWallToWorld(Vector3 wallSize, const Vector3& position, float x, float y, float z);
			GameObject* AddJailWallToWorld(Vector3 wallSize, const Vector3& position, float x, float y, float z);
			GameObject* AddChairToWorld(Vector3 wallSize, const Vector3& position, float x, float y, float z);
			GameObject* AddDeskToWorld(Vector3 wallSize, const Vector3& position, float x, float y, float z);
			GameObject* AddCoinToWorld(Vector3 wallSize, const Vector3& position, float x, float y, float z);
			GameObject* AddTableToWorld(Vector3 wallSize, const Vector3& position, float x, float y, float z);
			GameObject* AddComputerToWorld(Vector3 wallSize, const Vector3& position, float x, float y, float z);
			GameObject* AddFloorToWorld(Vector3 size, const Vector3& position);
			GameObject* AddBoxToWorld(const Vector3& position, Vector3 dimensions, const std::string name, float inverseMass = 10.0f);
			GameObject* AddButtonToWorld(Vector3 size, const Vector3& position, const std::string name, Mesh* mesh, Shader* shader, Texture* texture);
			GameObject* AddButtonnToWorld(ButtonTrigger* button, const Vector3& position, Door* linkedDoor);
			GameObject* AddPressableDoorToWorld(PressableDoor* door, Vector3 size, const Vector3& position, float x, float y, float z);
			GameObject* AddDoorToWorld(Door* door, Vector3 size, const Vector3& position, float x, float y, float z);
			GameObject* AddExitToWorld(Exit* exit, Vector3 size, const Vector3& position);
			GameObject* AddSoapToWorld(Soap* soap, Vector3 size, const Vector3& position);


			void AddHidingAreaToWorld(const Vector3& position, const Vector3& size, const std::string name);
			void LogObjectPlacement(const InGameObject& obj);
			void CreateWall(const InGameObject& obj);
			void CreateJailWall(const InGameObject& obj);
			void CreateChair(const InGameObject& obj);
			void CreateDesk(const InGameObject& obj);
			void CreateComputer(const InGameObject& obj);
			void CreateCoin(const InGameObject& obj);
			void CreateSoap(const InGameObject& obj);
			void CreateExit(const InGameObject& obj);
			void CreateTable(const InGameObject& obj);
			void CreateButton(const InGameObject& obj);
			void CreateDoorButton(const InGameObject& obj, std::unordered_map<std::string, Door*>& doorMap);
			void CreateBox(const InGameObject& obj);
			void CreateFloor(const InGameObject& obj);
			void CreateNormalDoor(const InGameObject& obj);
			Door* CreateButtonDoor(const InGameObject& obj);


			void CreateHidingArea(const InGameObject& obj);

			void DrawLoadingScreen();

			bool isMultiplayer;
			bool isServer;
			bool isPlaying;


		};
	}
}