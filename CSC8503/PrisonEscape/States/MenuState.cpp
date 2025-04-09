#include "GameTechRenderer.h"
#include "PushdownState.h"
#include <iostream>
#include "Window.h"
#include "Debug.h"
#include "imgui/imgui.h"
#include "GameServer.h"
#include "GameClient.h"

#include "PrisonEscape/States/MenuState.h"
#include "PrisonEscape/States/GameplayState.h"
#include "PrisonEscape/Core/GameSettingManager.h"
#include "PrisonEscape/Core/ImGuiManager.h"
#include "PrisonEscape/Core/GameConfigManager.h"
#include "PrisonEscape/Core/Networking/SteamManager.h"	
#include "PrisonEscape/Core/GameLevelManager.h"

using namespace NCL;
using namespace CSC8503;

MenuState::MenuState() :
	isConnecting(false),
	connectionStage(ConnectionStage::None),
	networkAsServer(false),
	connectionTimer(0.0f),
	connectionAttempt(0),
	gameConfig(nullptr),
	steamManager(nullptr)
{
	gameConfig = new GameConfigManager();

	gameConfig->steamInviteCallback = [this](uint64_t lobbyID) {
		HandleSteamInviteAccepted(lobbyID);
		};
}

MenuState::~MenuState()
{
	if (gameConfig)
	{
		delete gameConfig;
	}
}

void MenuState::OnAwake() {

	GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("MainMenuPanel", [this]() {	DrawMainMenuPanel(); });
	Debug::PrintDebugInfo({ "Menu State", Debug::RED });
	Window::GetWindow()->ShowOSPointer(true);
}

PushdownState::PushdownResult MenuState::OnUpdate(float dt, PushdownState** newState)
{
	// Connection Process Handling
	if (isConnecting)
	{
		connectionTimer += dt;

		switch (connectionStage)
		{
		case ConnectionStage::Creating:
			if (connectionTimer >= 1.5f) {
				// Using steam networking
				if (useSteamNetworking && steamManager && steamManager->IsInitialized()) {
					if (networkAsServer) {
						connectionStage = ConnectionStage::Success;
					}
					else {
						connectionStage = ConnectionStage::Connecting;
					}
					connectionTimer = 0.0f;
				}
				else {
					// Using regular networking
					connectionStage = ConnectionStage::Connecting;
					connectionTimer = 0.0f;

					gameConfig->networkConfig.isMultiplayer = true;
					gameConfig->networkConfig.isServer = networkAsServer;
					gameConfig->networkConfig.isUsingSteam = false;

					gameConfig->InitNetwork();

					if (networkAsServer) {
						try {
							gameConfig->CreateServer();
							connectionStage = ConnectionStage::Success;
							connectionTimer = 0.0f;
						}
						catch (std::exception& e) {
							delete gameConfig;
							this->gameConfig = nullptr;
							connectionStage = ConnectionStage::Failed;
							connectionTimer = 0.0f;
						}
					}
					else {
						try {
							gameConfig->CreateClient();
							connectionStage = ConnectionStage::Connecting;
							connectionTimer = 0.0f;
						}
						catch (std::exception& e) {
							delete gameConfig;
							this->gameConfig = nullptr;
							connectionStage = ConnectionStage::Failed;
							connectionTimer = 0.0f;
						}
					}
				}
			}
			break;

		case ConnectionStage::Connecting:
			if (networkAsServer) {
				// Server is already in "success" state from previous step
			}
			else {
				if (connectionTimer >= 1.0f) {
					connectionTimer = 0.0f;

					if (useSteamNetworking && steamManager && steamManager->IsInitialized()) {
						// For Steam clients, we're waiting on the lobby
						if (steamManager->IsInLobby()) {
							connectionStage = ConnectionStage::Success;
						}
						else {
							connectionAttempt++;
							if (connectionAttempt > 5) {
								connectionStage = ConnectionStage::Failed;
							}
						}
					}
					else if (gameConfig && gameConfig->networkConfig.client) {
						// Regular networking
						bool connected = false;
						try {
							uint8_t a, b, c, d;
							if (ParseIPAddress(gameConfig->networkConfig.ip, a, b, c, d) && gameConfig->networkConfig.client->Connect(a, b, c, d, NetworkBase::GetDefaultPort())) {
								connected = true;
								connectionStage = ConnectionStage::Success;
							}
							else {
								connectionAttempt++;
								if (connectionAttempt > 5) {
									connectionStage = ConnectionStage::Failed;
								}
							}
						}
						catch (std::exception& e) {
							connectionStage = ConnectionStage::Failed;
						}
					}
					else {
						// No gameConfig, this shouldn't happen
						connectionStage = ConnectionStage::Failed;
					}
				}
			}
			break;

		case ConnectionStage::Success:
			if (connectionTimer >= 2.0f) {
				connectionTimer = 0.0f;
				isConnecting = false;

				gameConfig->networkConfig.isMultiplayer = true;
				gameConfig->networkConfig.isServer = networkAsServer;
				gameConfig->networkConfig.isUsingSteam = useSteamNetworking;

				stateChangeAction = [this](PushdownState** newState) {
					if (gameConfig) {
						*newState = new GamePlayState(true, networkAsServer, gameConfig);
						dynamic_cast<GamePlayState*>(*newState)->SetGameConfig(gameConfig);
						gameConfig = nullptr; // Transfer ownership
					}
					};

				connectionStage = ConnectionStage::None;
			}
			break;

		case ConnectionStage::Failed:
			if (connectionTimer >= 3.0f) {
				isConnecting = false;
				connectionStage = ConnectionStage::None;

				if (gameConfig) {
					gameConfig->networkConfig.isMultiplayer = false;
				}

				if (useSteamNetworking) {
					GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("SteamLobbyPanel", [this]() {DrawSteamLobbyPanel(); });
				}
				else {
					GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("MultiplayerPanel", [this]() {DrawMultiplayerPanel(); });
				}
				GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("ConnectionPanel");
			}
			break;

		default:
			break;
		}
	}

	if (steamManager && steamManager->IsInitialized()) {
		gameConfig->SetSteamCallback();
		steamManager->Update();
	}

	if (stateChangeAction) {
		stateChangeAction(newState);
		stateChangeAction = nullptr;
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void MenuState::DrawMainMenuPanel() {
	std::vector<PanelButton> buttons = {
		{"Single Player", [this]() {
			std::cout << "Single Player" << std::endl;
			GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("LevelSelectPanel", [this]() {DrawLevelSelectPanel(); });
			GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("MainMenuPanel");

		}, 0.32, 0.25f},
		{"Multiplayer", [this]() {
			GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("MainMenuPanel");
			GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("MultiplayerPanel", [this]() {DrawMultiplayerPanel(); });
		}, 0.32f, 0.40f}, // Adjusted positions

		{"Settings", [this]() {
			Debug::PrintDebugInfo({ "Setting"});
			GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("SettingPanel", [this]() {DrawSettingPanel(); });
			GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("MainMenuPanel");
		}, 0.32f, 0.55f}, // Adjusted positions

		{"Quit Game", []() {
			std::cout << "Quitting game..." << std::endl;
			GameBase::GetGameBase()->QuitGame();
		}, 0.32f, 0.70f}  // Adjusted positions
	};

	ImGuiManager::DrawPanel("SLIPPPY SAM", buttons);
}


void MenuState::DrawLevelSelectPanel()
{
	std::vector<PanelButton> buttons = {
		{"Level One", [this]() {

				stateChangeAction = [this](PushdownState** newState) {
				gameConfig->networkConfig.isMultiplayer = false;

				if (this->gameConfig) {
					GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("LevelSelectPanel");
					gameConfig->SetChosenLevel("Level1");
					*newState = new GamePlayState(false, false, gameConfig);
					this->gameConfig = nullptr; // Transfer ownership

				}

			};

		}, 0.32f, .15f},
		{"Level Two", [this]() {

				stateChangeAction = [this](PushdownState** newState) {
				gameConfig->networkConfig.isMultiplayer = false;

				if (this->gameConfig) {
					GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("LevelSelectPanel");
					gameConfig->SetChosenLevel("Level2");
					*newState = new GamePlayState(false, false, gameConfig);
					this->gameConfig = nullptr; // Transfer ownership
				}

			};
		},0.32f, .30f},
		{"Level Three", [this]() {

				stateChangeAction = [this](PushdownState** newState) {
				gameConfig->networkConfig.isMultiplayer = false;

				if (this->gameConfig) {
					GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("LevelSelectPanel");
					gameConfig->SetChosenLevel("Level3");
					*newState = new GamePlayState(false, false, gameConfig);
					this->gameConfig = nullptr; // Transfer ownership
				}

			};
		},0.32f, .45f}
	};

	auto backCallback = [this]() {
		GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("MainMenuPanel", [this]() {DrawMainMenuPanel(); });
		GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("LevelSelectPanel");
		};

	ImGuiManager::DrawPanel("Select Level", buttons, {}, backCallback);
}

void MenuState::DrawSettingPanel() {
	std::vector<PanelButton> buttons = {
		{"Audio", [this]() {
				GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("AudioSettingPanel", [this]() {DrawAudioSettingPanel(); });
				GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("SettingPanel");
		}, 0.10f, .35f},
		{"Graphic", [this]() {
				GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("GraphicSettingPanel", [this]() {DrawVideoSettingPanel(); });
				GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("SettingPanel");
		},0.55f, .35f}
	};

	auto backCallback = [this]() {
		GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("MainMenuPanel", [this]() {DrawMainMenuPanel(); });
		GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("SettingPanel");
		};

	ImGuiManager::DrawPanel("Settings", buttons, {}, backCallback);
}

void MenuState::DrawAudioSettingPanel() {
	std::vector<PanelSlider> sliders = { {"Master Volume", &volume, 0, 100, 0.36f, 0.36f} };
	GameBase::GetGameSettings()->SetVolume(volume);

	auto backCallback = [this]() {
		GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("SettingPanel", [this]() {DrawSettingPanel(); });
		GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("AudioSettingPanel");
		};

	ImGuiManager::DrawPanel("Audio Settings", {}, sliders, backCallback);
}

void MenuState::DrawVideoSettingPanel() {
	std::vector<PanelSlider> sliders = { {"Brightness", &brightness, 0, 100, 0.36f, 0.36f} };
	std::vector<PanelCheckbox> checkboxes = {
	{ "VSync", &vSync, 0.36f, 0.56f } // Placed below the fullscreen checkbox
	};

	SetVsync(vSync);

	GameBase::GetGameSettings()->SetBrightness(brightness);
	auto backCallback = [this]() {
		GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("SettingPanel", [this]() {DrawSettingPanel(); });
		GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("GraphicSettingPanel");
		};

	ImGuiManager::DrawPanel("Video Settings", {}, sliders, backCallback, "", checkboxes);
}

void MenuState::DrawMultiplayerPanel() {
	std::vector<PanelButton> buttons = {
		{"Host", [this]() {
			networkAsServer = true;
			StartServerProcess();
		},0.32f, 0.35f},
		{"Join", [this]() {
			networkAsServer = false;
			StartClientProcess();
		},0.32f, 0.50f}
	};

	auto backCallback = [this]() {
		GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("MainMenuPanel", [this]() {DrawMainMenuPanel(); });
		GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("MultiplayerPanel");
		};

	ImGuiManager::DrawPanel("Multiplayer", buttons, {}, backCallback);

	ImVec2 windowSize = ImGui::GetWindowSize();

	ImGui::SetCursorPos(ImVec2(windowSize.x * 0.32f, windowSize.y * 0.2f));

	ImGui::PushFont(ImGuiManager::GetButtonFont());
	bool oldValue = useSteamNetworking;
	ImGui::Checkbox("Use Steam Networking", &useSteamNetworking);

	if (oldValue != useSteamNetworking) {
		if (useSteamNetworking) {
			steamManager = SteamManager::GetInstance();
			if (!steamManager->Initialize()) {
				GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("SteamErrorPanel", [this]() {
					ImGuiManager::DrawMessagePanel("Steam Error",
						"Failed to initialize Steam. Make sure Steam is running.",
						ImVec4(1, 0, 0, 1),
						[this]() {
							GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("SteamErrorPanel");
						});
					});
				useSteamNetworking = false;
				gameConfig->networkConfig.isUsingSteam = false;
			}
			else {
				std::cout << "Steam initialized successfully. User: " << steamManager->GetSteamUserName() << std::endl;
				gameConfig->networkConfig.isUsingSteam = true;
			}
		}
		else {
			if (steamManager) {
				steamManager->Shutdown();
				steamManager = nullptr;
				gameConfig->networkConfig.isUsingSteam = false;
			}
		}
	}

	// Display info text about Steam status
	if (useSteamNetworking && steamManager && steamManager->IsInitialized()) {
		ImGui::SetCursorPos(ImVec2(windowSize.x * 0.1f, windowSize.y * 0.27f));
		ImGui::TextColored(ImVec4(0, 1, 0, 1), "Steam user: %s", steamManager->GetSteamUserName().c_str());
	}

	ImGui::PopFont();
}

void MenuState::DrawJoinPanel()
{
	std::vector<PanelButton> buttons = {
		{"Connect", [this]() {
			// Parse the IP address
			uint8_t a, b, c, d;

			if (ParseIPAddress(ipAddressInput, a, b, c, d)) {
				// Store the IP for later use
				gameConfig->networkConfig.ip = ipAddressInput;

				// Start the connection process
				connectionStage = ConnectionStage::Creating;
				connectionAttempt = 1;
				connectionTimer = 0.0f;
				isConnecting = true;

				GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("ConnectionPanel", [this]() {
					DrawConnectionMessagePanel();
				});

				GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("JoinPanel");
			}
			else {
				// Show error message for invalid IP
				GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("IPErrorPanel", [this]() {
					ImGuiManager::DrawMessagePanel("Invalid IP Address",
						"Please enter a valid IP address in the format: xxx.xxx.xxx.xxx",
						ImVec4(1, 0, 0, 1),
						[this]() {
							GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("IPErrorPanel");
						});
				});
			}
		}, 0.32f, 0.50f}
	};

	auto backCallback = [this]() {
		GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("MultiplayerPanel", [this]() { DrawMultiplayerPanel(); });
		GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("JoinPanel");
		};

	ImGuiManager::DrawPanel("Join Server", buttons, {}, backCallback);

	// Add custom IP input field
	ImVec2 windowSize = ImGui::GetWindowSize();
	float startY = windowSize.y * 0.30f;
	ImGui::PushFont(ImGuiManager::GetButtonFont());
	ImGui::SetCursorPos(ImVec2(windowSize.x * 0.1f, startY));
	ImGui::Text("Server IP Address:");
	ImGui::SetCursorPos(ImVec2(windowSize.x * 0.1f, startY + 40));
	ImGui::SetNextItemWidth(windowSize.x * 0.8f);
	ImGui::InputText("##ipaddress", ipAddressInput, sizeof(ipAddressInput));
	ImGui::PopFont();
}

void MenuState::StartServerProcess()
{
	connectionStage = ConnectionStage::Creating;
	connectionTimer = 0.0f;
	isConnecting = true;

	gameConfig->networkConfig.isMultiplayer = true;
	gameConfig->networkConfig.isServer = true;
	gameConfig->networkConfig.isUsingSteam = useSteamNetworking;

	if (useSteamNetworking && steamManager && steamManager->IsInitialized()) {
		// Start a Steam lobby as host
		if (steamManager->CreateLobby(8)) {
			std::cout << "Creating Steam lobby..." << std::endl;
			connectionStage = ConnectionStage::Success;
		}
		else {
			std::cout << "Failed to create Steam lobby" << std::endl;
			useSteamNetworking = false;
			gameConfig->networkConfig.isUsingSteam = false;
		}
	}

	GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("ConnectionPanel", [this]() {DrawConnectionMessagePanel(); });
	GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("MultiplayerPanel");
}

void MenuState::StartClientProcess()
{
	gameConfig->networkConfig.isMultiplayer = true;
	gameConfig->networkConfig.isServer = false;
	gameConfig->networkConfig.isUsingSteam = useSteamNetworking;

	if (useSteamNetworking && steamManager && steamManager->IsInitialized()) {
		// For Steam as client, we need to show available lobbies or friend lobbies
		GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("SteamLobbyPanel", [this]() {DrawSteamLobbyPanel(); });
		GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("MultiplayerPanel");
		return;
	}

	GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("JoinPanel", [this]() { DrawJoinPanel(); });
	GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("MultiplayerPanel");
}

void MenuState::DrawConnectionMessagePanel()
{
	std::string title;
	std::string message;
	ImVec4 messageColor(1, 1, 1, 1);

	auto cancelCallback = [this]() {
		isConnecting = false;
		connectionStage = ConnectionStage::None;
		GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("MultiplayerPanel", [this]() {DrawMultiplayerPanel(); });
		GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("ConnectionPanel");

		};

	// Determine message based on current stage
	if (networkAsServer) {
		title = "Creating Server";

		switch (connectionStage) {
		case ConnectionStage::Creating:
			message = "Initializing server...";
			messageColor = ImVec4(1, 1, 0, 1); // Yellow
			break;

		case ConnectionStage::Connecting:
			message = "Setting up network...";
			messageColor = ImVec4(1, 1, 0, 1); // Yellow
			break;

		case ConnectionStage::Success:
			if (useSteamNetworking && steamManager && steamManager->IsInitialized()) {
				uint64_t lobbyID = steamManager->GetCurrentLobbyID();
				message = "Server created successfully!\n\nYour Lobby ID: " + std::to_string(lobbyID) + "\n\nShare this ID with friends to connect.";
			}
			else {
				message = "Server created successfully!";
			}
			messageColor = ImVec4(0, 1, 0, 1); // Green
			break;

		case ConnectionStage::Failed:
			message = "Server creation failed.";
			messageColor = ImVec4(1, 0, 0, 1); // Red
			break;

		default:
			message = "Unknown status";
			break;
		}
	}
	else {
		title = "Connecting to Server";

		switch (connectionStage) {
		case ConnectionStage::Creating:
			message = "Initializing connection...";
			messageColor = ImVec4(1, 1, 0, 1); // Yellow
			break;

		case ConnectionStage::Connecting:
			message = "Connection attempt " + std::to_string(connectionAttempt) + " of 5...";
			messageColor = ImVec4(1, 1, 0, 1); // Yellow
			break;

		case ConnectionStage::Success:
			message = "Connected to server successfully!";
			messageColor = ImVec4(0, 1, 0, 1); // Green
			break;

		case ConnectionStage::Failed:
			message = "Connection failed after multiple attempts.";
			messageColor = ImVec4(1, 0, 0, 1); // Red
			break;

		default:
			message = "Unknown status";
			break;
		}
	}

	// Don't show cancel button if we've already succeeded or failed
	if (connectionStage == ConnectionStage::Success || connectionStage == ConnectionStage::Failed) {
		ImGuiManager::DrawMessagePanel(title, message, messageColor);
	}
	else {
		ImGuiManager::DrawMessagePanel(title, message, messageColor, cancelCallback);
	}
}

void MenuState::InitializeSteam()
{
#ifdef ENABLE_STEAM
	// Initialize Steam manager
	steamManager = SteamManager::GetInstance();

	if (steamManager->Initialize())
	{
		std::cout << "Steam initialized successfully. User: " << steamManager->GetSteamUserName() << std::endl;

		// Set up Steam networking based on host/client status
		if (gameConfig->networkConfig.isServer)
		{
			// Create a Steam lobby
			steamManager->CreateLobby(8);

			gameConfig->networkConfig.isUsingSteam = true;

			connectionStage = ConnectionStage::Success;
			connectionTimer = 0.0f;
		}
	}
	else
	{
		gameConfig->networkConfig.isUsingSteam = false;
		std::cout << "Steam initialization failed. Using regular networking." << std::endl;
		delete steamManager;
		steamManager = nullptr;
	}
#endif
}

void MenuState::HandleSteamInvite(uint64_t friendSteamID)
{
	if (steamManager && steamManager->IsInitialized())
	{
		std::cout << "Sending game invite to friend: " << friendSteamID << std::endl;
		steamManager->SendGameInvite(friendSteamID);
	}
}

void MenuState::HandleSteamInviteAccepted(uint64_t lobbyID) {
	if (!steamManager || !steamManager->IsInitialized()) {
		return;
	}

	// Don't show invite notification to the host
	if (steamManager->IsLobbyOwner() && steamManager->GetCurrentLobbyID() == lobbyID) {
		// This is our own lobby - don't show an invite to ourselves
		return;
	}

	// Don't draw Invite Panel f already drawn
	if (drawInvitePanel)
	{
		return;
	}

	// Show notification about the invite
	GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("InviteAcceptedPanel", [this, lobbyID]() { DrawInviteAcceptedPanel(lobbyID); });
}

void MenuState::DrawSteamLobbyPanel() {
	// Get the Steam manager instance
	SteamManager* steamManager = SteamManager::GetInstance();

	if (!steamManager || !steamManager->IsInitialized()) {
		// Steam not available - fallback to regular networking
		useSteamNetworking = false;
		gameConfig->networkConfig.isUsingSteam = false;
		StartClientProcess();
		return;
	}

	// Create buttons for back functionality
	std::vector<PanelButton> buttons;

	// Add a back button
	auto backCallback = [this]() {
		GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("MultiplayerPanel", [this]() { DrawMultiplayerPanel(); });
		GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("SteamLobbyPanel");
		};

	// Custom drawing for the lobby list
	ImVec2 windowSize = ImGui::GetWindowSize();
	float startY = windowSize.y * 0.25f; // Start below the header

	// Get the list of friends from Steam who might be in a lobby
	std::vector<std::pair<std::string, uint64_t>> onlineFriends;

	// Filter for friends who are online
	std::vector<std::pair<std::string, uint64_t>> allFriends = steamManager->GetFriendsList();
	for (const auto& friendInfo : allFriends) {
		if (steamManager->IsFriendOnline(friendInfo.second)) {
			onlineFriends.push_back(friendInfo);
		}
	}

	ImGui::PushFont(ImGuiManager::GetButtonFont());

	// First, draw "direct connect" option
	{
		ImGui::SetCursorPos(ImVec2(windowSize.x * 0.1f, startY));
		ImGui::Text("Direct Connect To Lobby ID:");

		static char lobbyIdInput[64] = "";
		ImGui::SetCursorPos(ImVec2(windowSize.x * 0.1f, startY + 40));
		ImGui::SetNextItemWidth(windowSize.x * 0.5f);
		ImGui::InputText("##lobbyid", lobbyIdInput, sizeof(lobbyIdInput));

		ImGui::SetCursorPos(ImVec2(windowSize.x * 0.65f, startY + 40));
		if (ImGui::Button("Connect", ImVec2(windowSize.x * 0.25f, 30))) {
			// Try to parse and connect to the lobby ID
			try {
				uint64_t lobbyId = std::stoull(lobbyIdInput);
				JoinSteamLobby(lobbyId);
			}
			catch (std::exception& e) {
				std::cout << "Invalid lobby ID: " << e.what() << std::endl;

				// Show error message
				GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("LobbyErrorPanel", [this]() {
					ImGuiManager::DrawMessagePanel("Invalid Lobby ID",
						"The lobby ID entered is not valid. Please check and try again.",
						ImVec4(1, 0, 0, 1),
						[this]() {
							GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("LobbyErrorPanel");
						});
					});
			}
		}
	}

	// Section header for friends
	ImGui::SetCursorPos(ImVec2(windowSize.x * 0.1f, startY + 100));
	ImGui::TextColored(ImVec4(0, 1, 1, 1), "Friends in Lobbies:");

	float itemHeight = windowSize.y * 0.08f;
	float padding = windowSize.y * 0.02f;
	float currentY = startY + 140;

	if (onlineFriends.empty()) {
		ImGui::SetCursorPos(ImVec2(windowSize.x * 0.2f, currentY));
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "No online friends found");
	}
	else {
		// Draw each friend with a join button
		for (const auto& friendInfo : onlineFriends) {
			const std::string& friendName = friendInfo.first;
			uint64_t friendID = friendInfo.second;

			// Check if this friend is in a game/lobby
			bool isInGame = steamManager->IsFriendInGame(friendID);

			// Friend name
			ImGui::SetCursorPos(ImVec2(windowSize.x * 0.1f, currentY));
			ImGui::Text("%s", friendName.c_str());

			// Status indicator 
			ImGui::SetCursorPos(ImVec2(windowSize.x * 0.5f, currentY));
			if (isInGame) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "In Game"); // Green status
			}
			else {
				ImGui::TextColored(ImVec4(0, 0.7f, 1, 1), "Online"); // Blue status
			}

			currentY += itemHeight + padding;
		}
	}

	ImGui::PopFont();
}

void MenuState::JoinSteamLobby(uint64_t lobbyID) {
	if (!steamManager || !steamManager->IsInitialized()) {
		return;
	}

	// Start the connection process
	connectionStage = ConnectionStage::Creating;
	connectionTimer = 0.0f;
	isConnecting = true;

	// Try to join the Steam lobby
	if (steamManager->JoinLobby(lobbyID)) {
		std::cout << "Joining Steam lobby: " << lobbyID << std::endl;
		connectionStage = ConnectionStage::Success;
	}
	else {
		std::cout << "Failed to join Steam lobby" << std::endl;
		connectionStage = ConnectionStage::Failed;
	}

	GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("ConnectionPanel", [this]() {DrawConnectionMessagePanel(); });
	GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("SteamLobbyPanel");
}

bool MenuState::ParseIPAddress(const std::string& ipString, uint8_t& a, uint8_t& b, uint8_t& c, uint8_t& d)
{
	std::stringstream ss(ipString);
	int tempA, tempB, tempC, tempD;
	char dot;

	if (ss >> tempA >> dot >> tempB >> dot >> tempC >> dot >> tempD) {
		// Check valid ranges for IP components (0-255)
		if (tempA >= 0 && tempA <= 255 &&
			tempB >= 0 && tempB <= 255 &&
			tempC >= 0 && tempC <= 255 &&
			tempD >= 0 && tempD <= 255) {

			a = static_cast<uint8_t>(tempA);
			b = static_cast<uint8_t>(tempB);
			c = static_cast<uint8_t>(tempC);
			d = static_cast<uint8_t>(tempD);
			return true;
		}
	}
	return false;
}

void MenuState::DrawInviteAcceptedPanel(uint64_t lobbyID)
{
	drawInvitePanel = true;

	ImGuiManager::DrawPopupPanel("Game Invitation",
		"You've been invited to join a game. Would you like to connect?",
		ImVec4(0, 1, 0, 1),
		[this, lobbyID]() {
			GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("InviteAcceptedPanel");
			GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("MultiplayerPanel");
			JoinSteamLobby(lobbyID);
		},
		[this]() {
			// Decline
			GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("InviteAcceptedPanel");
		},
		"Join Game", "Decline");
}

void MenuState::SetVsync(bool vsync)
{
	vSync = vsync;
	GameBase::GetGameBase()->GetRenderer()->SetVerticalSync(vsync ? VerticalSyncState::VSync_ON : VerticalSyncState::VSync_OFF);
}

