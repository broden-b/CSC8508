#pragma once

#include "GameState.h"

namespace NCL {
	namespace CSC8503 {
		class PushdownState;
		class GameConfigManager;
		class SteamManager;

		class MenuState : public GameState {
		public:
			MenuState();
			~MenuState();

			PushdownResult OnUpdate(float dt, PushdownState** newState) override;
			void OnAwake() override;

			void DrawMainMenuPanel();
			void DrawLevelSelectPanel();
			void DrawSettingPanel();
			void DrawAudioSettingPanel();
			void DrawVideoSettingPanel();
			void DrawMultiplayerPanel();
			void DrawJoinPanel();
			void DrawSteamLobbyPanel();
			void DrawInviteAcceptedPanel(uint64_t lobbyID);

			// Connection handling methods
			void StartServerProcess();
			void StartClientProcess();
			void DrawConnectionMessagePanel();

			void InitializeSteam();
			void HandleSteamInvite(uint64_t friendSteamID);
			void HandleSteamInviteAccepted(uint64_t lobbyID);
			void JoinSteamLobby(uint64_t lobbyID);
			void SetVsync(bool vsync);

		private:
			enum class ConnectionStage {
				None,
				Creating,
				Connecting,
				Success,
				Failed
			};

			std::function<void(PushdownState**)> stateChangeAction = nullptr;
			int volume = 50;
			int brightness = 50;
			bool fullscreen = false;
			bool vSync = false;

			// Network connection state
			char ipAddressInput[64] = "127.0.0.1";
			bool isConnecting = false;
			ConnectionStage connectionStage;
			bool networkAsServer;
			float connectionTimer;
			int connectionAttempt;
			GameConfigManager* gameConfig;

			bool ParseIPAddress(const std::string& ipString, uint8_t& a, uint8_t& b, uint8_t& c, uint8_t& d);

			SteamManager* steamManager;
			bool useSteamNetworking = false;
			bool drawInvitePanel = false;
		};
	}
}