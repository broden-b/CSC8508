#pragma once

#include "GameState.h"

namespace NCL {
	namespace Rendering {
		class Texture;
	}
}

namespace NCL {
	namespace CSC8503 {
		class PushdownState;
		class GameConfigManager;
		class GameLevelManager;
		class Level;


		class GamePlayState : public GameState {
		public:
			GamePlayState(bool multiplayer, bool asServer, GameConfigManager* config);
			~GamePlayState();

			void OnAwake() override;
			PushdownResult OnUpdate(float dt, PushdownState** newState) override;
			void SetGameConfig(GameConfigManager* config);

			void DrawHUDPanel();
			void DrawFriendsPanel();
			void DrawFriendsListWindow(const std::vector<std::pair<std::string, uint64_t>>& onlineFriends);

			bool FriendsPanelVisible() const { return friendsPanelVisible; }

		private:
			GameLevelManager* manager;
			GameConfigManager* gameConfig;
			bool friendsPanelVisible = false;

			void InitializeSteamMultiplayer(Level* level);
			bool InitializeSteamNetworking();
			void InitializeMultiplayer(Level* level);
			void InitializeSinglePlayer(Level* level);

			void SetupServer(Level* level);
			void SetupClient(Level* level);
			void SetupClientPlayer(Level* level);

			void RegisterServerPacketHandlers();
			void RegisterClientPacketHandlers();

			NCL::Rendering::Texture* heartFilledTexture = nullptr;
			NCL::Rendering::Texture* heartEmptyTexture = nullptr;

			int score = 0;

		public:
			GameConfigManager* GetGameConfig() const { return gameConfig; }
		};
	}
}