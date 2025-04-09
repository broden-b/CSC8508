#pragma once

#include "../CSC8503/PrisonEscape/Core/AudioManager.h"
#include "PhysicsSystem.h"
#include "../CSC8503/PrisonEscape/Scripts/Player/Player.h"

namespace NCL {
	namespace CSC8503 {
		class Coin : public GameObject {
		public:
			Coin(const std::string& name);
			~Coin();

		protected:
			int value = 1;
			bool collected;
		};
	}
}