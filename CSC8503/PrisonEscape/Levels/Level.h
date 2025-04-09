#pragma once

#include "PhysicsSystem.h"
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"
#include "imgui/imgui.h"
#include "NetworkBase.h"

namespace NCL {
	namespace CSC8503 {
		class PatrolEnemy;
		class PursuitEnemy;

		class Player;
		class Level : public PacketReceiver
		{
		public:
			Level();
			~Level() {}
			virtual void Update(float dt);

			virtual void Init();
			void AddPatrolEnemyToLevel(PatrolEnemy* patrolEnemy);
			void AddPursuitEnemyToLevel(PursuitEnemy* pursuitEnemy);
			void AddPlayerToLevel(Player* player, const Vector3& position);
			void ReceivePacket(int type, GamePacket* packet, int source) override;

		protected:
			Mesh* cubeMesh = nullptr;
			Mesh* kittenMesh = nullptr;
			Texture* basicTex = nullptr;
			Texture* pauseButton = nullptr;
			Shader* basicShader = nullptr;

			Player* playerOne = nullptr;
			Player* playerTwo = nullptr;
			std::vector<PatrolEnemy*> patrolEnemies;
			std::vector<PursuitEnemy*> pursuitEnemies;
			Vector3 playerOneSpawn;

		private:
			void SetCameraAttributes();

			GameObject* AddFloorToWorld(const Vector3& position, const Vector3& floorSize, const Vector4& color);
			GameObject* AddMeshToWorldPosition(const Vector3& position, Mesh* mesh, const Vector3& meshSize, VolumeType type, const Vector3& volumeSize, const float& inverseMass = 10.0f);

		public:
			Player* GetPlayerOne() const { return playerOne; }
			Player* GetPlayerTwo() const { return playerTwo; }

#pragma region UI

		public:
			void DrawPauseButton();
			void DrawPauseMenu();

#pragma endregion

		};
	}
}
