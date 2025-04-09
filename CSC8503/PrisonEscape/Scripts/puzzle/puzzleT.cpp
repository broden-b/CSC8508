#include "PuzzleT.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "../CSC8503/PrisonEscape/Core/GameLevelManager.h"

using namespace NCL;
using namespace CSC8503;

NormalCollidCube::NormalCollidCube(const std::string& name) : GameObject("CollidingCube") {
	AABBVolume* volume = new AABBVolume(Vector3(1, 1, 1));
	SetBoundingVolume((CollisionVolume*)volume);
}

NormalCollidCube::~NormalCollidCube() {
}

void NormalCollidCube::OnCollisionBegin(GameObject* otherObject) {
	RenderObject* renderObject = GetRenderObject();
	if (renderObject && (otherObject->GetName() == "playerOne" || otherObject->GetName() == "playerTwo")) {
		static int colorIndex = 0;
		std::vector<Vector4> colors = {
			Vector4(1, 0, 0, 1), // Red
			Vector4(0, 0, 1, 1), // Blue
			Vector4(0, 1, 0, 1), // Green
			Vector4(1, 1, 1, 1), // White
			Vector4(0, 0, 0, 1), // Black
			Vector4(1, 1, 0, 1)  // Yellow
		};
		renderObject->SetColour(colors[colorIndex]);
		colorIndex = (colorIndex + 1) % colors.size();
	}
}

Door::Door() : GameObject("Door") {
	isOpen = false;
	openTexture = nullptr;
	closeTexture = nullptr;
	
	audioManager = new AudioManager();
	audioManager->Initialize();
	audioManager->LoadSoundsXtension();
}
void Door::Open() {
	isOpen = true;
	GetRenderObject()->SetDefaultTexture(openTexture);
	if (!audioManager->IsPlaying(audioManager->soundFile6)) { // Ensure sound isn't already playing
		audioManager->PlaySound(audioManager->soundFile6);

	}

	Vector3 newPosition = GetTransform().GetPosition() + Vector3(0, -15, 0); 
	GetTransform().SetPosition(newPosition);
	
}

void Door::Close() {
	isOpen = false;
	GetRenderObject()->SetDefaultTexture(closeTexture);
}
void Door::SetTextures(Texture* closedTex, Texture* openTex)
{
	closeTexture = closedTex;
	openTexture = openTex;
}

void Door::Update(float dt) {
	if (isOpen && GetRenderObject()->GetDefaultTexture() != openTexture) {
		GetRenderObject()->SetDefaultTexture(openTexture);
	}
	else if (!isOpen && GetRenderObject()->GetDefaultTexture() != closeTexture) {
		GetRenderObject()->SetDefaultTexture(closeTexture);
	}
}


ButtonTrigger::ButtonTrigger(const std::string& name) : GameObject(name) {
	isPressed = false;
	linkedDoor = nullptr;

}


void ButtonTrigger::OnCollisionBegin(GameObject* otherObject) {
	if (!isPressed && (otherObject->GetName() == "playerOne" || otherObject->GetName() == "playerTwo")) {
		isPressed = true;
		GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));  // Change color to green (activated)

		if (linkedDoor) {
			linkedDoor->Open();// Open the door
			
			
			
		}
	}
}

void ButtonTrigger::SetLinkedDoor(Door* door) {
	linkedDoor = door;
}

PressableDoor::PressableDoor() : Door() {
	isPressed = false;
}

void PressableDoor::OnCollisionBegin(GameObject* otherObject) {
	if (!isPressed && (otherObject->GetName() == "playerOne" || otherObject->GetName() == "playerTwo")) {
		isPressed = true;
		Open();
	}
}

Exit::Exit() {}



void Exit::OnCollisionBegin(GameObject* otherObject) {
	if (dynamic_cast<Player*>(otherObject)) {
		GameLevelManager* levelManager = GameLevelManager::GetGameLevelManager();
		if (levelManager) {
			levelManager->ClearLevel(); 
			std::cout << "player name: " + levelManager->GetPlayerOne()->GetName() + "\n";
			levelManager->loadMap("level2.json"); 
		}
	}
}
Soap::Soap() {}
void Soap::OnCollisionBegin(GameObject* otherObject) {
	
	Player* player = dynamic_cast<Player*>(otherObject);
	if (player) {
		player->sprintMultiplier += 7.5f;  // Increase sprint speed by 50%
		std::cout << "Player's sprint speed increased!" << std::endl;
		GameBase::GetGameBase()->GetWorld()->RemoveGameObject(this);
		player->SetScore(player->GetScore() + 250);
	}
	audioManager = new AudioManager();
	audioManager->Initialize();
	audioManager->LoadSoundsXtension();
	audioManager->PlaySound(audioManager->soundFile12);
}