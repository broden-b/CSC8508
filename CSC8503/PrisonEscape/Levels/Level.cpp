#include "Level.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"
#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"
#include "GameTechRenderer.h"
#include "../CSC8503/PrisonEscape/Core/GameConfigManager.h"
#include "PrisonEscape/Core/GameBase.h"
#include "PrisonEscape/Scripts/PatrolEnemy/PatrolEnemy.h"
#include "PrisonEscape/Scripts/Player/Player.h"
#include "PrisonEscape/Scripts/PursuitEnemy/PursuitEnemy.h"



using namespace NCL;
using namespace CSC8503;

Level::Level()
{

}

void Level::Init()
{
	SetCameraAttributes();
	//GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("PauseButton", [this]() {DrawPauseButton();});
	GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("MainMenuPanel");
}

void Level::Update(float dt)
{
	GameBase::GetGameBase()->GetWorld()->GetMainCamera().UpdateCamera(dt);
}

void Level::ReceivePacket(int type, GamePacket* payload, int source) {
	if (type == Player_Position) {
		PlayerPositionPacket* posPacket = (PlayerPositionPacket*)payload;

		GameConfigManager* config = GameBase::GetGameBase()->GetGameConfig();

		// Update the appropriate player based on ID
		if (posPacket->playerID == 1) {
			if (playerOne && config && !config->networkConfig.isServer) {
				playerOne->GetTransform().SetPosition(Vector3(posPacket->posX, posPacket->posY, posPacket->posZ));
			}
		}
		else {
			if (playerTwo && config && config->networkConfig.isServer) {
				playerTwo->GetTransform().SetPosition(Vector3(posPacket->posX, posPacket->posY, posPacket->posZ));
			}
		}
	}
	else if (type == Player_ID_Assignment) {
		PlayerIDPacket* idPacket = (PlayerIDPacket*)payload;

		// Store the player ID in the config
		GameConfigManager* config = GameBase::GetGameBase()->GetGameConfig();
		if (config) {
			config->networkConfig.playerID = idPacket->playerID;
		}
	}
}

void Level::AddPlayerToLevel(Player* player, const Vector3& position)
{
	if (player->GetName() == "playerOne")
	{
		this->playerOne = player;

		playerOneSpawn = position;
		playerOne->SetSpawn(playerOneSpawn);
		this->playerOne->GetRenderObject()->GetTransform()->SetPosition(playerOneSpawn);

	}
	else if (player->GetName() == "playerTwo")
	{
		this->playerTwo = player;
		this->playerTwo->GetRenderObject()->GetTransform()->SetPosition(position);
	}
}

void Level::AddPatrolEnemyToLevel(PatrolEnemy* patrolEnemy)
{
	if (patrolEnemy) {
		patrolEnemies.push_back(patrolEnemy);
		patrolEnemy->GetRenderObject()->GetTransform()->SetPosition(Vector3(50, 5, 50));
	}
}

void NCL::CSC8503::Level::AddPursuitEnemyToLevel(PursuitEnemy* pursuitEnemy)
{
	if (pursuitEnemy) {
		pursuitEnemies.push_back(pursuitEnemy);
		pursuitEnemy->GetRenderObject()->GetTransform()->SetPosition(Vector3(40, 5, 40));
	}
}


void Level::SetCameraAttributes()
{
	GameBase::GetGameBase()->GetWorld()->GetMainCamera().SetNearPlane(0.1f);
	GameBase::GetGameBase()->GetWorld()->GetMainCamera().SetFarPlane(500.0f);
	GameBase::GetGameBase()->GetWorld()->GetMainCamera().SetPitch(-35.0f);
	GameBase::GetGameBase()->GetWorld()->GetMainCamera().SetYaw(0.0f);
}

GameObject* Level::AddFloorToWorld(const Vector3& position, const Vector3& floorSize, const Vector4& color) {
	GameObject* floor = new GameObject();

	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);

	floor->GetTransform().SetScale(floorSize * 2.0f).SetPosition(position);

	RenderObject* renderObject = new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader);
	renderObject->SetColour(color);
	floor->SetRenderObject(renderObject);
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));
	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	GameBase::GetGameBase()->GetWorld()->AddGameObject(floor);

	return floor;
}

GameObject* Level::AddMeshToWorldPosition(const Vector3& position, Mesh* mesh, const Vector3& meshSize, VolumeType type, const Vector3& volumeSize, const float& inverseMass)
{
	GameObject* object = new GameObject();

	CollisionVolume* volume = nullptr;

	switch (type)
	{
	case NCL::VolumeType::AABB:
		volume = new AABBVolume(volumeSize);
		break;
	case NCL::VolumeType::OBB:
		volume = new OBBVolume(volumeSize);
		break;
	case NCL::VolumeType::Sphere:
		volume = new SphereVolume(volumeSize.x);
		break;
	case NCL::VolumeType::Mesh:

		break;
	case NCL::VolumeType::Capsule:
		volume = new CapsuleVolume(volumeSize.x, volumeSize.y);
		break;
	case NCL::VolumeType::Compound:
		break;
	case NCL::VolumeType::Invalid:

		break;
	default:
		break;
	}
	object->SetBoundingVolume((CollisionVolume*)volume);

	object->GetTransform().SetScale(volumeSize).SetPosition(position);

	RenderObject* renderObject = new RenderObject(&object->GetTransform(), mesh, basicTex, basicShader);

	object->SetRenderObject(renderObject);
	object->SetPhysicsObject(new PhysicsObject(&object->GetTransform(), object->GetBoundingVolume()));

	switch (type)
	{
	case NCL::VolumeType::AABB:
		object->GetPhysicsObject()->InitCubeInertia();
		break;
	case NCL::VolumeType::OBB:
		object->GetPhysicsObject()->InitSphereInertia();
		break;
	case NCL::VolumeType::Sphere:
		object->GetPhysicsObject()->InitSphereInertia();
		break;
	case NCL::VolumeType::Mesh:
		//maybe define later
		break;
	case NCL::VolumeType::Capsule:
		object->GetPhysicsObject()->InitSphereInertia();
		break;
	case NCL::VolumeType::Compound:
		//maybe define later
		break;
	case NCL::VolumeType::Invalid:

		break;
	default:
		break;
	}
	object->GetPhysicsObject()->SetInverseMass(inverseMass);

	GameBase::GetGameBase()->GetWorld()->AddGameObject(object);

	return object;
}





#pragma region UI

//void Level::DrawPauseButton()
//{
//	ImVec2 windowSize = ImGui::GetWindowSize();
//	ImGui::SetCursorPos(ImVec2(windowSize.x * .9f, windowSize.y * .05f));
//	GLuint texID = ((OGLTexture*)pauseButton)->GetObjectID();
//
//	if (ImGui::ImageButton("Test", texID, ImVec2(100, 100)))
//	{
//		GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("PauseMenu", [this]() {DrawPauseMenu();});
//	}
//}

void Level::DrawPauseMenu()
{
	std::cout << "Pause Menu" << std::endl;
}
#pragma endregion
