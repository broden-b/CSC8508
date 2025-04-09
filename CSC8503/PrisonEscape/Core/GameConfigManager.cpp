#include "GameServer.h"
#include "GameClient.h"

#include "PrisonEscape/Core/GameConfigManager.h"
#include "PrisonEscape/Core/Networking/SteamManager.h"

using namespace NCL;
using namespace CSC8503;

GameConfigManager::GameConfigManager()
{
	std::cout << "Created Manager" << std::endl;
}

GameConfigManager::~GameConfigManager()
{
	if (networkConfig.server)
	{
		std::cout << "Shutting down server...\n";
		networkConfig.server->Shutdown();
		delete networkConfig.server;
	}

	if (networkConfig.client)
	{
		std::cout << "Disconnecting client...\n";
		delete networkConfig.client;
	}
}

void GameConfigManager::InitNetwork()
{
	if (networkConfig.isMultiplayer)
	{
		static bool networkInitialized = false;
		if (!networkInitialized) {
			std::cout << "Initializing network system...\n";
			NetworkBase::Initialise();
			networkInitialized = true;
			std::cout << "Network system initialized\n";
		}
	}
}

void GameConfigManager::CreateServer()
{
	try
	{
		std::cout << "Creating server...\n";
		networkConfig.server = new GameServer(NetworkBase::GetDefaultPort(), 8);
		networkConfig.playerID = 1;  // Server is always player 1
		std::cout << "Server started successfully on port " << NetworkBase::GetDefaultPort() << "\n";
	}
	catch (const std::exception& e)
	{
		std::cout << "Server creation failed: " << e.what() << "\n";
		delete networkConfig.server;
		networkConfig.server = nullptr;
		networkConfig.isMultiplayer = false;
		networkConfig.isServer = false;
		std::cout << "Falling back to single player mode\n";
	}
}

void GameConfigManager::CreateClient()
{
	try {
		std::cout << "Creating client...\n";
		networkConfig.client = new GameClient();
		if (!networkConfig.client->netHandle) {
			throw std::runtime_error("Failed to initialize client network handle");
		}

		networkConfig.playerID = 0;
	}
	catch (const std::exception& e) {
		std::cout << "Client connection failed: " << e.what() << "\n";
		delete networkConfig.client;
		networkConfig.client = nullptr;
		networkConfig.isMultiplayer = false;
		std::cout << "Falling back to single player mode\n";
	}
}

void GameConfigManager::SetSteamCallback()
{
	SteamManager* steamManager = SteamManager::GetInstance();
	if (steamManager && steamManager->IsInitialized()) {
		steamManager->SetJoinGameCallback([this](uint64_t lobbyID) {
			if (steamInviteCallback) {
				steamInviteCallback(lobbyID);
			}
		});
	}
}
