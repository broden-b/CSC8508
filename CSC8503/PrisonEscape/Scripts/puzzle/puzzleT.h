#pragma once

#include "GameObject.h"
#include "GameWorld.h"
#include "../CSC8503/PrisonEscape/Core/AudioManager.h"
namespace NCL {
	namespace CSC8503 {

		class NormalCollidCube : public GameObject {
		public:
			NormalCollidCube(const std::string& name = "");
			~NormalCollidCube();
			void OnCollisionBegin(GameObject* otherObject) override;
		};
		
		class Door : public GameObject {
		public:
			Door();
			~Door();

			void Update(float dt) ;
			void Open();
			void Close();
			bool IsOpen() const { return isOpen; }
			void SetTextures(Texture* closedTex, Texture* openTex);
			
		private:
			bool isOpen;
			Texture* openTexture;
			Texture* closeTexture;
			AudioManager* audioManager;
		};

		class ButtonTrigger : public GameObject {
		public:
			ButtonTrigger(const std::string& name = "ButtonTrigger");
			~ButtonTrigger() = default;

			void OnCollisionBegin(GameObject* otherObject) override;
			void SetLinkedDoor(Door* door);

		private:
			bool isPressed;
			Door* linkedDoor;
			
		};

		class PressableDoor : public Door {
		public:
			PressableDoor();
			~PressableDoor() = default;

			void OnCollisionBegin(GameObject* otherObject) override;

		private:
			bool isPressed;
		};

		class Exit : public GameObject {
		public:
			Exit();
			~Exit() = default;

			void OnCollisionBegin(GameObject* otherObject) override;
			
		};

		class Soap : public GameObject {
		public:
			Soap();
			~Soap() = default;
			void OnCollisionBegin(GameObject* otherObject) override;
		private:
			AudioManager* audioManager;
		};
	}
}