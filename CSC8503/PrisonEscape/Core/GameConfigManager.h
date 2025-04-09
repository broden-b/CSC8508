#pragma once

namespace NCL {
	namespace CSC8503 {
		class GameServer;
		class GameClient;

		class GameConfigManager {
		private:
			struct NetworkConfig {
				bool isMultiplayer = false;
				bool isServer = false;
				std::string ip = "127.0.0.1";
				GameServer* server = nullptr;
				GameClient* client = nullptr;
				bool isUsingSteam = false;
				int playerID;
			};

		public:
			GameConfigManager();
			~GameConfigManager();

			void InitNetwork();
			void CreateServer();
			void CreateClient();

			void SetChosenLevel(std::string levelName) { chosenLevel = levelName; }
			std::string GetChosenLevel() { return chosenLevel; }


			NetworkConfig networkConfig;

			void SetSteamCallback();
			std::function<void(uint64_t)> steamInviteCallback;

			std::string chosenLevel;
		};
	}
}