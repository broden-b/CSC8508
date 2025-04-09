#pragma once

#include "../CSC8503/PrisonEscape/Core/GameLevelManager.h"
#include "PushdownState.h"
#include "GameWorld.h"
#include "Window.h"
#include "Debug.h"
#include "PhysicsSystem.h"
#include "GameState.h"
#include "imgui/imgui.h"


namespace NCL {
	namespace CSC8503 {
		class GameConfigManager;

		enum ButtonClicked
		{
			None,
			Resume,
			Invite,
			Settings,
			Exit
		};

		class PauseState : public GameState {
		public:
			PauseState(GameConfigManager* gameConfig);
			void OnAwake() override;

			PushdownResult OnUpdate(float dt, PushdownState** newState) override;

		protected:
			void DrawPauseMenuPanel();
			void DrawSettingPanel();
			void DrawAudioSettingPanel();
			void DrawVideoSettingPanel();
			void DrawFriendsPanel();
			std::function<void(PushdownState**)> stateChangeAction;

		private:
			int volume = 50;
			int brightness = 50;

			ButtonClicked buttonClicked = ButtonClicked::None;

			GameConfigManager* gameConfig;

		};
	}
}
