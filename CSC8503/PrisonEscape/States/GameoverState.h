#pragma once
#include "GameState.h"
#include "PrisonEscape/Core/ImGuiManager.h"

namespace NCL {
	namespace CSC8503 {
		enum class GameOverReason {
			TimeUp,
			GameOver,
			OutOfLives
		};
		class GameOverState : public GameState {

		public:
			GameOverState(GameOverReason reason) : gameOverReason(reason) {}
			void OnAwake() override;
			PushdownResult OnUpdate(float dt, PushdownState** newState) override;

		protected:
			std::function<void(PushdownState**)> stateChangeAction;
			void DrawGameOverPanel();
			GameOverReason gameOverReason;
		};
	}
}