#include "GameTechRenderer.h"
#include "PauseState.h"
#include "GamePlayState.h"
#include "MenuState.h"
#include <iostream>
#include "PrisonEscape/Core/GameSettingManager.h"
#include "PrisonEscape/Core/GameBase.h"
#include "PrisonEscape/Core/ImGuiManager.h"
#include "PrisonEscape/Core/GameConfigManager.h"
#include "PrisonEscape/Core/Networking/SteamManager.h"

using namespace NCL;
using namespace CSC8503;

PauseState::PauseState(GameConfigManager* gameConfig)
{
	this->gameConfig = gameConfig;
}

void PauseState::OnAwake() {
	GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("PauseMenuPanel", [this]() { DrawPauseMenuPanel(); });
	GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("HUDPanel");
	Window::GetWindow()->ShowOSPointer(true);
}

PushdownState::PushdownResult PauseState::OnUpdate(float dt, PushdownState** newState) {
	if (buttonClicked == ButtonClicked::Resume) {
		*newState = nullptr;
		return PushdownResult::Pop;
	}
	else if (buttonClicked == ButtonClicked::Settings) {
		return PushdownResult::NoChange;
	}
	else if (buttonClicked == ButtonClicked::Exit) {
		*newState = new MenuState;
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void PauseState::DrawPauseMenuPanel() {

	// Initialize buttons vector with the first button
	std::vector<PanelButton> buttons = {
		{"Resume", [this]() {
			this->buttonClicked = ButtonClicked::Resume;
			GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("PauseMenuPanel");
		}, .32f, 0.25f}
	};


	//
	//if (gameConfig && gameConfig->networkConfig.isUsingSteam) {
	//	buttons.push_back({
	//		"Invite Friend", [this]() {
	//			this->buttonClicked = ButtonClicked::Invite;
	//			GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("FriendsPanel", [this]() { DrawFriendsPanel(); });
	//			GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("PauseMenuPanel");
	//		}, .32f, 0.40f  // Adjust position if needed
	//	});
	//}

	// Add remaining buttons
	buttons.push_back({
		"Settings", [this]() {
			this->buttonClicked = ButtonClicked::Settings;
			GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("PauseSettingPanel", [this]() { DrawSettingPanel(); });
			GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("PauseMenuPanel");
		}, .32f, gameConfig && gameConfig->networkConfig.isUsingSteam ? 0.55f : 0.45f
		});

	buttons.push_back({
		"Quit Game", [this]() {
		GameBase::GetGameBase()->QuitGame();
		}, .32f, gameConfig && gameConfig->networkConfig.isUsingSteam ? 0.70f : 0.65f
		});

	ImGuiManager::DrawPanel("Pause Menu", buttons);
}

void PauseState::DrawSettingPanel() {
	std::vector<PanelButton> buttons = {
		{"Audio", [this]() {
			GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("PauseAudioSettingPanel", [this]() { DrawAudioSettingPanel(); });
			GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("PauseSettingPanel");
		},0.10f, 0.35f},
		{"Graphics", [this]() {
			GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("PauseGraphicSettingPanel", [this]() { DrawVideoSettingPanel(); });
			GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("PauseSettingPanel");
		},0.55f, 0.35f}
	};

	auto backCallback = [this]() {
		GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("PauseMenuPanel", [this]() { DrawPauseMenuPanel(); });
		GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("PauseSettingPanel");
		};

	ImGuiManager::DrawPanel("Settings", buttons, {}, backCallback);
}

void PauseState::DrawAudioSettingPanel() {
	std::vector<PanelSlider> sliders = { {"Master Volume", &volume, 0, 100,0.36f, 0.36f} };
	GameBase::GetGameSettings()->SetVolume(volume);
	auto backCallback = [this]() {
		GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("PauseSettingPanel", [this]() {DrawSettingPanel(); });
		GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("PauseAudioSettingPanel");
		};

	ImGuiManager::DrawPanel("Audio Settings", {}, sliders, backCallback);
}

void PauseState::DrawVideoSettingPanel() {
	std::vector<PanelSlider> sliders = { {"Brightness", &brightness, 0, 100, 0.36f, 0.36f} };
	GameBase::GetGameSettings()->SetBrightness(brightness);
	auto backCallback = [this]() {
		GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("PauseSettingPanel", [this]() { DrawSettingPanel(); });
		GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("PauseGraphicSettingPanel");
		};

	ImGuiManager::DrawPanel("Video Settings", {}, sliders, backCallback);
}

//void PauseState::DrawFriendsPanel()
//{
//	// Get the Steam manager instance
//	SteamManager* steamManager = SteamManager::GetInstance();
//
//	if (!steamManager || !steamManager->IsInitialized()) {
//		// Steam not available - show error message
//		ImGuiManager::DrawMessagePanel("Steam Error", "Steam is not initialized. Please restart the game and make sure Steam is running.", ImVec4(1, 0, 0, 1));
//		return;
//	}
//
//	// Get the list of friends from Steam
//	std::vector<std::pair<std::string, uint64_t>> allFriends = steamManager->GetFriendsList();
//
//	// Filter online friends only
//	std::vector<std::pair<std::string, uint64_t>> onlineFriends;
//	for (const auto& friendInfo : allFriends) {
//		if (steamManager->IsFriendOnline(friendInfo.second) && steamManager->DoesFriendOwnGame(friendInfo.second)) {
//			onlineFriends.push_back(friendInfo);
//		}
//	}
//
//	// Create buttons for back functionality
//	std::vector<PanelButton> buttons;
//
//	// Add a back button
//	auto backCallback = [this]() {
//		GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("PauseMenuPanel", [this]() { DrawPauseMenuPanel(); });
//		GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("FriendsPanel");
//		};
//
//	ImGuiManager::DrawPanel("Online Friends", buttons, {}, backCallback, "Select a friend to invite");
//
//	// Custom drawing for the friends list since we need more complex layout
//	ImVec2 windowSize = ImGui::GetWindowSize();
//	float startY = windowSize.y * 0.25f; // Start below the header
//
//	if (gameConfig && gameConfig->networkConfig.isUsingSteam) {
//		uint64_t lobbyID = steamManager->GetCurrentLobbyID();
//		if (lobbyID != 0) {
//			ImGui::SetCursorPos(ImVec2(windowSize.x * 0.1f, startY));
//			ImGui::TextColored(ImVec4(0, 1, 1, 1), "Your Lobby ID: %llu", lobbyID);
//			startY += 50; // Move down a bit for the rest of the content
//		}
//	}
//
//	if (onlineFriends.empty()) {
//		// No online friends found
//		ImGui::SetCursorPos(ImVec2(windowSize.x * 0.2f, startY));
//		ImGui::TextColored(ImVec4(1, 1, 0, 1), "No online friends found who own the game");
//	}
//	else {
//		// Draw each friend with an invite button
//		float itemHeight = windowSize.y * 0.08f;
//		float padding = windowSize.y * 0.02f;
//		float currentY = startY;
//
//		ImGui::PushFont(ImGuiManager::GetButtonFont());
//
//		for (const auto& friendInfo : onlineFriends) {
//			const std::string& friendName = friendInfo.first;
//			uint64_t friendID = friendInfo.second;
//
//			// Friend name
//			ImGui::SetCursorPos(ImVec2(windowSize.x * 0.1f, currentY));
//			ImGui::Text("%s", friendName.c_str());
//
//			// Status indicator (green circle for online)
//			ImGui::SetCursorPos(ImVec2(windowSize.x * 0.5f, currentY + itemHeight * 0.5f));
//			ImGui::TextColored(ImVec4(0, 1, 0, 1), "?"); // Green dot for online
//
//			// Invite button
//			ImGui::SetCursorPos(ImVec2(windowSize.x * 0.6f, currentY));
//			std::string buttonLabel = "Invite##" + std::to_string(friendID);
//
//			if (ImGui::Button(buttonLabel.c_str(), ImVec2(windowSize.x * 0.25f, itemHeight))) {
//				// Send invite to this friend
//				steamManager->SendGameInvite(friendID);
//
//				// Show confirmation message
//				GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("InviteSentPanel", [friendName, this]() {
//					ImGuiManager::DrawMessagePanel("Invite Sent",
//						"Game invite sent to " + friendName,
//						ImVec4(0, 1, 0, 1),
//						[this]() {
//							GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("InviteSentPanel");
//							GameBase::GetGameBase()->GetRenderer()->AddPanelToCanvas("FriendsPanel", [this]() { DrawFriendsPanel(); });
//						});
//					});
//
//				GameBase::GetGameBase()->GetRenderer()->DeletePanelFromCanvas("FriendsPanel");
//			}
//
//			currentY += itemHeight + padding;
//		}
//
//		ImGui::PopFont();
//	}
//}