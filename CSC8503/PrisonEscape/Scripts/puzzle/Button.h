#pragma once

#include "PhysicsSystem.h"
#include "Window.h"
#include "Vector.h"
#include "Mesh.h"
#include "../CSC8503/PrisonEscape/Core/GameBase.h"
#include "../CSC8503/PrisonEscape/Core/AudioManager.h"

namespace NCL {
	namespace CSC8503 {
		class Button : public GameObject {
		public:
			Button();
			~Button();

			void SetPlayerActivated(bool state) { playerActivated = state; }
			void SetBoxActivated(bool state) { boxActivated = state; }
			void setButtonObject(GameObject* object) { buttonObject = object; }

			bool IsPlayerActivated() const { return playerActivated; }
			bool IsBoxActivated() const { return boxActivated; }
			bool IsPressed() const { return pressed; }

			void pressDetection(const std::vector<GameObject*>& boxes/*, GameObject* player*/);

		private:
			bool pressed;
			bool playerActivated;
			bool boxActivated;
			AudioManager* audioManager;

			// "animate" the button that gets pressed down (move it down lower so it seems like its been pressed)
			void pressDownButton();


			GameObject* buttonObject = nullptr;

		};
	}
}