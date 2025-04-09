#include "Button.h"
#include "CollisionVolume.h"
#include "CollisionDetection.h"


Button::Button() {
	pressed = false;
	playerActivated = false;
	boxActivated = false;
	audioManager = new AudioManager();
	audioManager->Initialize();
	audioManager->LoadSoundsXtension();
}

Button::~Button() {
	buttonObject = nullptr;
	delete audioManager;
}

void Button::pressDownButton() {
	if (!pressed && buttonObject) {
		buttonObject->GetTransform().SetPosition(buttonObject->GetTransform().GetPosition() + Vector3(0, -1.5, 0));
		pressed = true;
		audioManager->PlaySound(audioManager->soundFile2);
	}
}


void Button::pressDetection(const std::vector<GameObject*>& boxes/*, GameObject* player*/) {
	CollisionDetection::CollisionInfo info;

	for (GameObject* box : boxes) {
		if (box && CollisionDetection::ObjectIntersection(this->buttonObject, box, info)) {
			if (boxActivated) {
				pressDownButton();
				return;
			}
		}
	}
}
