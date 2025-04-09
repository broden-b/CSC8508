#include "GameTechRenderer.h"
#include "PushdownState.h"
#include "GameWorld.h"
#include "Window.h"
#include "Texture.h"
#include "GameServer.h"
#include "GameClient.h"

#include "PrisonEscape/States/GameplayState.h"
#include "PrisonEscape/States/GameoverState.h"
#include "PrisonEscape/States/PauseState.h"
#include "PrisonEscape/Core/GameConfigManager.h"
#include "PrisonEscape/Core/GameLevelManager.h"
#include "PrisonEscape/Scripts/PatrolEnemy/PatrolEnemy.h"
#include "PrisonEscape/Core/Networking/SteamManager.h"
#include "PrisonEscape/Scripts/PursuitEnemy/PursuitEnemy.h"


using namespace NCL;
using namespace CSC8503;

GamePlayState::GamePlayState(bool multiplayer, bool asServer, GameConfigManager* config)
{
	this->gameConfig = config;
	manager = new GameLevelManager(GameBase::GetGameBase()->GetWorld(), GameBase::GetGameBase()->GetRenderer(), config->GetChosenLevel(), gameConfig->networkConfig.isMultiplayer, gameConfig->networkConfig.isServer);
	Level* level = new Level();
	level->Init();
	manager->AddLevel(level);
	manager->SetCurrentLevel(level);

	bool useSteamNetworking = InitializeSteamNetworking();

	if (gameConfig->networkConfig.isMultiplayer) {
		if (useSteamNetworking) {
			InitializeSteamMultiplayer(level);
		}
		else {
			InitializeMultiplayer(level);
		}
	}
	else {
		InitializeSinglePlayer(level);
	}

	heartFilledTexture = GameBase::GetGameBase()->GetRenderer()->LoadTexture("heart_filled.png");
	heartEmptyTexture = GameBase::GetGameBase()->GetRenderer()->LoadTexture("heart_empty.png");
}

void GamePlayState::OnAwake()
{
	GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("ConnectionPanel");

	GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("HUDPanel", [this]() { DrawHUDPanel(); });

	Window::GetWindow()->LockMouseToWindow(true);
	Window::GetWindow()->ShowOSPointer(false);
}

GamePlayState::~GamePlayState()
{
	delete manager;
	delete gameConfig;
}

PushdownState::PushdownResult GamePlayState::OnUpdate(float dt, PushdownState** newState)
{
	SteamManager* steamManager = SteamManager::GetInstance();
	bool useSteamNetworking = steamManager && steamManager->IsInitialized();

	if (gameConfig->networkConfig.isMultiplayer) {

		if (useSteamNetworking)
		{
			Level* level = manager->GetCurrentLevel();
			if (level)
			{
				Player* playerToSync = steamManager->IsLobbyOwner() ? manager->GetPlayerOne() : manager->GetPlayerTwo();
				if (playerToSync) {
					Vector3 pos = playerToSync->GetTransform().GetPosition();
					steamManager->SendPlayerPosition(pos);
				}
			}
			steamManager->Update();
		}
		else
		{
			if (gameConfig->networkConfig.isServer)
			{
				// Server: Send PlayerOne position to clients
				Level* level = manager->GetCurrentLevel();
				if (level && manager->GetPlayerOne())
				{
					Vector3 pos = manager->GetPlayerOne()->GetTransform().GetPosition();
					PlayerPositionPacket posPacket(1, pos.x, pos.y, pos.z);
					gameConfig->networkConfig.server->SendGlobalPacket(posPacket);
				}

				gameConfig->networkConfig.server->UpdateServer();
			}
			else if (gameConfig->networkConfig.client)
			{
				// Client: Send PlayerTwo position to server
				Level* level = manager->GetCurrentLevel();
				if (level && manager->GetPlayerTwo())
				{
					Vector3 pos = manager->GetPlayerTwo()->GetTransform().GetPosition();
					PlayerPositionPacket posPacket(gameConfig->networkConfig.playerID, pos.x, pos.y, pos.z);
					gameConfig->networkConfig.client->SendPacket(posPacket);
				}

				gameConfig->networkConfig.client->UpdateClient();
			}
		}
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::P) && !gameConfig->networkConfig.isMultiplayer)
	{
		*newState = new PauseState(gameConfig);
		return PushdownResult::Push;
	}
	manager->UpdateGame(dt);

	if (manager->GetPlayerOne()->GetHealth() == 0)
	{
		*newState = new GameOverState(GameOverReason::OutOfLives);
		return PushdownResult::Push;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::TAB) && gameConfig->networkConfig.isMultiplayer && gameConfig->networkConfig.isUsingSteam)
	{
		friendsPanelVisible = !friendsPanelVisible;
		if (friendsPanelVisible)
		{
			GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("FriendsPanel", [this]() { DrawFriendsPanel(); });
		}
		else
		{
			GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("FriendsPanel");
		}
	}



	return PushdownResult::NoChange;
}

void GamePlayState::SetGameConfig(GameConfigManager* config)
{
	if (gameConfig != nullptr && gameConfig != config) {
		delete gameConfig;
	}
	gameConfig = config;
	GameBase::GetGameBase()->SetGameConfig(gameConfig);
}

void GamePlayState::DrawHUDPanel() {
	// Set cursor to the top-left corner
	ImGui::SetCursorPos(ImVec2(25, 25));

	// Define the remaining lives and max number of hearts (e.g., 5 hearts)
	int remainingLives = manager->GetPlayerOne()->GetHealth(); // Number of filled hearts
	int maxLives = 3;       // Maximum number of hearts to display

	GLuint texIDFilled = ((OGLTexture*)heartFilledTexture)->GetObjectID();  // Filled heart texture
	GLuint texIDUnfilled = ((OGLTexture*)heartEmptyTexture)->GetObjectID(); // Unfilled heart texture

	// Calculate the size of the heart icon
	ImVec2 heartSize = ImVec2(40, 40); // Size of each heart

	// Initial position for hearts
	ImVec2 heartPos = ImGui::GetCursorScreenPos(); // This gets the current position for the heart drawing

	// Loop through to draw the hearts
	for (int i = 0; i < maxLives; i++) {
		ImVec2 heartPosition = ImVec2(heartPos.x + (i * heartSize.x), heartPos.y); // Adjust position for each heart
		ImVec2 heartEndPos = ImVec2(heartPosition.x + heartSize.x, heartPosition.y + heartSize.y); // Set the end position based on heart size

		// Draw filled heart for remaining lives
		if (i < remainingLives) {
			ImGui::GetWindowDrawList()->AddImage((ImTextureID)texIDFilled, heartPosition, heartEndPos); // Draw filled heart
		}
		// Draw unfilled heart for remaining hearts
		else {
			ImGui::GetWindowDrawList()->AddImage((ImTextureID)texIDUnfilled, heartPosition, heartEndPos); // Draw unfilled heart
		}

		// Debugging: output the position of each heart
	}

	// Set next cursor position for Timer text
	ImGui::SetCursorPos(ImVec2(25, 75)); // Set cursor to below the hearts

	// Display Score
	ImGui::Text(("Score: " + std::to_string(manager->GetPlayerOne()->GetScore())).c_str(), 20.0f);



}
void GamePlayState::InitializeSteamMultiplayer(Level* level)
{
	SteamManager* steamManager = SteamManager::GetInstance();
	bool isHost = steamManager->IsLobbyOwner();

	// Create both players regardless of host status
	Player* playerOne = manager->GetPlayerOne();
	playerOne->InitializeController();

	Player* playerTwo = manager->GetPlayerTwo();
	playerTwo->InitializeController();

	// Determine which player this client controls based on Steam role
	if (isHost)
	{
		level->AddPlayerToLevel(playerOne, manager->GetP1Position());
		level->AddPlayerToLevel(playerTwo, manager->GetP2Position());
		// Position playerTwo off-screen initially until connected
		playerTwo->GetTransform().SetPosition(Vector3(10, -100, 10));
	}
	else
	{
		level->AddPlayerToLevel(playerTwo, manager->GetP2Position());
		level->AddPlayerToLevel(playerOne, manager->GetP1Position());
		// Position playerOne off-screen initially
		playerOne->GetTransform().SetPosition(Vector3(10, -100, 10));
	}

	// Set up camera for the active player
	Player* activePlayer = isHost ? playerOne : playerTwo;
	Vector3 playerPosition = activePlayer->GetTransform().GetPosition();
	GameBase::GetGameBase()->GetWorld()->GetMainCamera().SetPosition(Vector3(playerPosition.x, playerPosition.y, playerPosition.z));

	// Register with Steam for position updates
	steamManager->SetPlayerUpdateCallback([this, steamManager, playerOne, playerTwo](uint64_t steamID, const Vector3& pos) {
		// Update the appropriate player based on Steam ID
		if (steamManager->IsLobbyOwner())
		{
			// Host updates playerTwo
			playerTwo->GetTransform().SetPosition(pos);
		}
		else
		{
			// Client updates playerOne
			playerOne->GetTransform().SetPosition(pos);
		}
		});
}

bool GamePlayState::InitializeSteamNetworking() {
	SteamManager* steamManager = SteamManager::GetInstance();
	bool useSteamNetworking = false;
	if (steamManager && steamManager->IsInitialized()) {
		useSteamNetworking = true;
		std::cout << "Using Steam for networking" << std::endl;
	}
	return useSteamNetworking;
}

void GamePlayState::InitializeMultiplayer(Level* level) {
	if (gameConfig->networkConfig.isServer && gameConfig->networkConfig.server) {
		SetupServer(level);
	}
	else if (gameConfig->networkConfig.client) {
		SetupClient(level);
	}
}

void GamePlayState::InitializeSinglePlayer(Level* level) {
	std::cout << "Single player mode..." << std::endl;

	// Create and initialize player one (single-player)
	Player* player = manager->GetPlayerOne();
	player->InitializeController();
	level->AddPlayerToLevel(player, manager->GetP1Position());

	// Set the camera position for the single-player
	Vector3 playerPosition = level->GetPlayerOne()->GetTransform().GetPosition();
	GameBase::GetGameBase()->GetWorld()->GetMainCamera().SetPosition(Vector3(playerPosition.x, playerPosition.y, playerPosition.z));
}

void GamePlayState::SetupServer(Level* level) {
	std::cout << "Setting up server..." << std::endl;

	// Create and initialize player one (server)
	Player* playerOne = manager->GetPlayerOne();
	playerOne->InitializeController();
	level->AddPlayerToLevel(playerOne, manager->GetP1Position());

	// Set the camera position for the server player
	Vector3 playerPosition = level->GetPlayerOne()->GetTransform().GetPosition();
	GameBase::GetGameBase()->GetWorld()->GetMainCamera().SetPosition(Vector3(playerPosition.x, playerPosition.y, playerPosition.z));

	// Set up server callbacks for client connections
	gameConfig->networkConfig.server->SetPlayerConnectedCallback([this, level](int playerID) {
		SetupClientPlayer(level);
		});

	// Register packet handlers for server
	RegisterServerPacketHandlers();
}

void GamePlayState::SetupClient(Level* level) {
	std::cout << "Setting up client..." << std::endl;

	// Create and initialize both players (client-side)
	Player* playerOne = manager->GetPlayerOne();
	playerOne->InitializeController();

	Player* playerTwo = manager->GetPlayerTwo();
	playerTwo->InitializeController();

	level->AddPlayerToLevel(playerOne, manager->GetP1Position());
	level->AddPlayerToLevel(playerTwo, manager->GetP2Position());

	// Set the initial camera position for the client player
	Vector3 playerPosition = manager->GetP2Position();
	GameBase::GetGameBase()->GetWorld()->GetMainCamera().SetPosition(Vector3(playerPosition.x, playerPosition.y, playerPosition.z));

	// Register packet handlers for the client
	RegisterClientPacketHandlers();
}

void GamePlayState::SetupClientPlayer(Level* level) {
	Player* playerTwo = this->manager->GetPlayerTwo();
	playerTwo->InitializeController();
	level->AddPlayerToLevel(playerTwo, manager->GetP2Position());
}

void GamePlayState::RegisterServerPacketHandlers() {
	gameConfig->networkConfig.server->RegisterPacketHandler(Player_ID_Assignment, manager->GetCurrentLevel());
	gameConfig->networkConfig.server->RegisterPacketHandler(Player_Position, manager->GetCurrentLevel());
}

void GamePlayState::RegisterClientPacketHandlers() {
	gameConfig->networkConfig.client->RegisterPacketHandler(Player_Position, manager->GetCurrentLevel());
	gameConfig->networkConfig.client->RegisterPacketHandler(Player_ID_Assignment, manager->GetCurrentLevel());
}

void GamePlayState::DrawFriendsPanel()
{
	Window::GetWindow()->ShowOSPointer(true);
	// Get the Steam manager instance
	SteamManager* steamManager = SteamManager::GetInstance();

	if (!steamManager || !steamManager->IsInitialized()) {
		// Steam not available - show error message
		ImGuiManager::DrawMessagePanel("Steam Error", "Steam is not initialized. Please restart the game and make sure Steam is running.", ImVec4(1, 0, 0, 1));
		return;
	}

	// Get the list of friends from Steam
	std::vector<std::pair<std::string, uint64_t>> allFriends = steamManager->GetFriendsList();

	// Filter online friends only
	std::vector<std::pair<std::string, uint64_t>> onlineFriends;
	for (const auto& friendInfo : allFriends) {
		if (steamManager->IsFriendOnline(friendInfo.second) && steamManager->DoesFriendOwnGame(friendInfo.second)) {
			onlineFriends.push_back(friendInfo);
		}
	}

	Window::GetWindow()->ShowOSPointer(false);
	DrawFriendsListWindow(onlineFriends);
}

void GamePlayState::DrawFriendsListWindow(const std::vector<std::pair<std::string, uint64_t>>& onlineFriends)
{
	Window::GetWindow()->ShowOSPointer(true);
	// Get the screen size
	ImVec2 screenSize = ImGui::GetIO().DisplaySize;

	// Set the desired window size
	ImVec2 windowSize(600, 400); // You can adjust the size as per your needs

	// Calculate the position to center the window
	ImVec2 windowPos = ImVec2((screenSize.x - windowSize.x) * 0.5f, (screenSize.y - windowSize.y) * 0.5f);

	// Set the next window position and size before calling Begin()
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

	if (ImGui::Begin("Friends List", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {

		// Custom drawing for the friends list
		ImVec2 windowSize = ImGui::GetWindowSize();
		float startY = windowSize.y * 0.25f; // Start below the header

		SteamManager* steamManager = SteamManager::GetInstance();
		if (gameConfig && gameConfig->networkConfig.isUsingSteam) {
			uint64_t lobbyID = steamManager->GetCurrentLobbyID();
			if (lobbyID != 0) {
				ImGui::SetCursorPos(ImVec2(windowSize.x * 0.1f, startY));
				ImGui::TextColored(ImVec4(0, 1, 1, 1), "Your Lobby ID: %llu", lobbyID);
				startY += 50; // Move down a bit for the rest of the content
			}
		}

		if (onlineFriends.empty()) {
			// No online friends found
			ImGui::SetCursorPos(ImVec2(windowSize.x * 0.2f, startY));
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "No online friends found who own the game");
		}
		else {
			// Draw each friend with an invite button
			float itemHeight = windowSize.y * 0.08f;
			float padding = windowSize.y * 0.02f;
			float currentY = startY;

			ImGui::PushFont(ImGuiManager::GetButtonFont());

			for (const auto& friendInfo : onlineFriends) {
				const std::string& friendName = friendInfo.first;
				uint64_t friendID = friendInfo.second;

				// Status indicator (green circle for online)
				ImGui::SetCursorPos(ImVec2(windowSize.x * 0.1f, currentY));
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "."); // Green dot for online

				// Friend name
				ImGui::SetCursorPos(ImVec2(windowSize.x * 0.2f, currentY));
				ImGui::Text("%s", friendName.c_str());

				// Invite button
				ImGui::SetCursorPos(ImVec2(windowSize.x * 0.6f, currentY));
				std::string buttonLabel = "Invite##" + std::to_string(friendID);

				if (ImGui::Button(buttonLabel.c_str(), ImVec2(windowSize.x * 0.25f, itemHeight))) {
					// Send invite to this friend
					steamManager->SendGameInvite(friendID);

					// Show confirmation message
					GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("InviteSentPanel", [friendName, this]() {
						ImGuiManager::DrawMessagePanel("Invite Sent", "Game invite sent to " + friendName, ImVec4(0, 1, 0, 1));
						});

					GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("InviteSentPanel");
					GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("FriendsPanel");
				}

				currentY += itemHeight + padding;
			}

			ImGui::PopFont();
		}
		Window::GetWindow()->ShowOSPointer(false);
		ImGui::End(); // End the ImGui window
	}
}
