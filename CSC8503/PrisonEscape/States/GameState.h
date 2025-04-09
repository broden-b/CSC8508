#pragma once

#include "PushdownState.h"
#include "PrisonEscape/Core/GameBase.h"

namespace NCL {
	namespace CSC8503 {
		class GameState : public PushdownState {
		public:
			GameState() {}
			virtual ~GameState() {}
		};
	}
}