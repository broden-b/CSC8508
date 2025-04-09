#include <string>
#include <vector>
#include <functional>
#include "Texture.h"

#include "PrisonEscape/Core/ImGuiManager.h"
#include "PrisonEscape/Core/GameBase.h"
#include "PrisonEscape/States/MenuState.h"

using namespace NCL;
using namespace CSC8503;

ImFont* ImGuiManager::buttonFont = nullptr;
ImFont* ImGuiManager::headerFont = nullptr;
ImFont* ImGuiManager::footerFont = nullptr;
ImFont* ImGuiManager::messageFont = nullptr;


void ImGuiManager::Initialize() {

	ImGuiIO& imguiIO = ImGui::GetIO();
	buttonFont = imguiIO.Fonts->AddFontFromFileTTF("../Assets/Fonts/BebasNeue-Regular.ttf", 30.0f, NULL, imguiIO.Fonts->GetGlyphRangesDefault());
	headerFont = imguiIO.Fonts->AddFontFromFileTTF("../Assets/Fonts/BebasNeue-Regular.ttf", 60.0f, NULL, imguiIO.Fonts->GetGlyphRangesDefault());
	footerFont = imguiIO.Fonts->AddFontFromFileTTF("../Assets/Fonts/BebasNeue-Regular.ttf", 30.0f, NULL, imguiIO.Fonts->GetGlyphRangesDefault());
	messageFont = imguiIO.Fonts->AddFontFromFileTTF("../Assets/Fonts/BebasNeue-Regular.ttf", 24.0f, NULL, imguiIO.Fonts->GetGlyphRangesDefault());
	imguiIO.Fonts->Build();
}
void ImGuiManager::DrawPanel(
	const std::string& title,
	const std::vector<PanelButton>& buttons,
	const std::vector<PanelSlider>& sliders,
	std::function<void()> backCallback,
	const std::string& footer, const std::vector<PanelCheckbox>& checkboxes
) {
	ImVec2 windowSize = ImGui::GetWindowSize();

	DrawHeader(title);

	for (const auto& button : buttons) {
		DrawButton(button.label, button.callback, button.xPosition, button.yPosition);
	}

	for (const auto& slider : sliders) {
		DrawSlider(slider.label, slider.value, slider.min, slider.max, slider.xPosition, slider.yPosition);
	}

	for (auto& checkbox : checkboxes) {  // Checkbox rendering
		DrawCheckbox(checkbox.label, checkbox.checked, checkbox.xPosition, checkbox.yPosition);
	}

	if (backCallback) {
		DrawBackButton(backCallback);
	}

	DrawFooter(footer);
}


void ImGuiManager::DrawMessagePanel(
	const std::string& title,
	const std::string& message,
	const ImVec4& messageColor,
	std::function<void()> cancelCallback
) {
	ImVec2 windowSize = ImGui::GetWindowSize();


	DrawHeader(title);

	ImGui::PushFont(messageFont);

	ImVec2 textSize = ImGui::CalcTextSize(message.c_str());
	ImGui::SetCursorPos(ImVec2(
		(windowSize.x - textSize.x) * 0.5f,
		windowSize.y * 0.4f
	));

	ImGui::TextColored(messageColor, "%s", message.c_str());
	ImGui::PopFont();


	float time = ImGui::GetTime();
	float radius = 25.0f;
	ImVec2 center = ImVec2(windowSize.x * 0.5f, windowSize.y * 0.5f);

	ImDrawList* draw_list = ImGui::GetWindowDrawList();


	const int num_segments = 12;
	const float PI = 3.14159265358979323846f;
	float start = time * 5.0f;

	for (int i = 0; i < num_segments; i++) {
		float a = start + (i * 2 * PI / num_segments);
		float b = start + ((i + 1) * 2 * PI / num_segments);
		float alpha = 1.0f - (float)i / (float)num_segments;

		draw_list->AddLine(
			ImVec2(center.x + cosf(a) * radius, center.y + sinf(a) * radius),
			ImVec2(center.x + cosf(b) * radius, center.y + sinf(b) * radius),
			ImGui::GetColorU32(ImVec4(messageColor.x, messageColor.y, messageColor.z, alpha)),
			3.0f
		);
	}

	if (cancelCallback) {
		ImGui::PushFont(buttonFont);
		ImGui::SetCursorPos(ImVec2(windowSize.x * 0.35f, windowSize.y * 0.65f));

		if (ImGui::Button("Cancel", ImVec2(windowSize.x * BUTTON_WIDTH_RATIO, windowSize.y * BUTTON_HEIGHT_RATIO))) {
			cancelCallback();
		}

		ImGui::PopFont();
	}
}

void ImGuiManager::DrawPopupPanel(const std::string& title, const std::string& message,
	const ImVec4& messageColor, std::function<void()> acceptCallback,
	std::function<void()> declineCallback,
	const std::string& acceptText, const std::string& declineText) {
	ImVec2 screenSize = ImGui::GetIO().DisplaySize;
	ImVec2 windowSize(400, 200);
	ImVec2 windowPos = ImVec2((screenSize.x - windowSize.x) * 0.5f, (screenSize.y - windowSize.y) * 0.5f);

	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

	ImGui::Begin(title.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);

	// Display the message
	ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
	ImGui::TextColored(messageColor, "%s", message.c_str());
	ImGui::PopTextWrapPos();

	// Calculate button positions
	float buttonWidth = windowSize.x * 0.3f;
	float buttonSpacing = windowSize.x * 0.1f;
	float buttonsY = windowSize.y * 0.7f;

	if (acceptCallback && declineCallback) {
		// Two buttons mode
		ImGui::SetCursorPos(ImVec2((windowSize.x - 2 * buttonWidth - buttonSpacing) * 0.5f, buttonsY));
		if (ImGui::Button(acceptText.c_str(), ImVec2(buttonWidth, 0))) {
			if (acceptCallback) acceptCallback();
		}

		ImGui::SetCursorPos(ImVec2((windowSize.x - 2 * buttonWidth - buttonSpacing) * 0.5f + buttonWidth + buttonSpacing, buttonsY));
		if (ImGui::Button(declineText.c_str(), ImVec2(buttonWidth, 0))) {
			if (declineCallback) declineCallback();
		}
	}
	else if (acceptCallback) {
		// Single button mode
		ImGui::SetCursorPos(ImVec2((windowSize.x - buttonWidth) * 0.5f, buttonsY));
		if (ImGui::Button(acceptText.c_str(), ImVec2(buttonWidth, 0))) {
			if (acceptCallback) acceptCallback();
		}
	}

	ImGui::End();
}

void ImGuiManager::DrawHeader(const std::string& title) {
	if (title.empty()) {
		return;
	}

	ImVec2 windowSize = ImGui::GetWindowSize();
	ImVec2 textSize = ImGui::CalcTextSize(title.c_str());


	ImVec2 textPos = ImVec2((windowSize.x - textSize.x) * 0.47f, windowSize.y * 0.05f); // Adjust vertical position as needed

	ImGui::PushFont(headerFont);
	ImGui::SetCursorPos(textPos);
	ImGui::TextColored(ImVec4(0, 1, 1, 1), "%s", title.c_str());
	ImGui::PopFont();


}

void ImGuiManager::DrawButton(const std::string& label, const std::function<void()>& callback, float xPos, float yPos) {

	// Set cursor position using the new x and y positions
	//nEED TO FIND A WAY TO LOAD tEXTURE HERE THIS WON'T WORL
	/*Texture* tex = GameBase::GetGameBase()->GetRenderer()->LoadTexture("button.png");
	GLuint texID = ((OGLTexture*)tex)->GetObjectID();*/
	ImVec2 windowSize = ImGui::GetWindowSize();
	ImGui::PushFont(buttonFont);
	ImGui::SetCursorPos(ImVec2(windowSize.x * xPos, windowSize.y * yPos));



	ImGui::SetCursorPos(ImVec2(windowSize.x * xPos, windowSize.y * yPos));
	if (ImGui::Button(label.c_str(), ImVec2(windowSize.x * BUTTON_WIDTH_RATIO, windowSize.y * BUTTON_HEIGHT_RATIO))) {
		if (callback) {
			callback();
		}
	}
	ImGui::PopFont();
}


void ImGuiManager::DrawSlider(const std::string& label, int* value, int min, int max, float horizontalPos, float verticalPos) {
	ImVec2 windowSize = ImGui::GetWindowSize();


	ImGui::SetCursorPos(ImVec2(windowSize.x * horizontalPos, windowSize.y * verticalPos));
	ImGui::Text("%s", label.c_str());
	ImGui::SameLine();
	ImGui::SetCursorPos(ImVec2(windowSize.x * (horizontalPos + 0.15f), windowSize.y * verticalPos)); // Slight offset for slider
	ImGui::SetNextItemWidth(windowSize.x * 0.2f);
	ImGui::SliderInt(("##" + label).c_str(), value, min, max);
}


void ImGuiManager::DrawBackButton(const std::function<void()>& callback) {
	ImVec2 windowSize = ImGui::GetWindowSize();
	ImGui::PushFont(buttonFont);

	ImGui::SetCursorPos(ImVec2(windowSize.x * 0.32f, windowSize.y * .65f));

	if (ImGui::Button("Back", ImVec2(windowSize.x * BUTTON_WIDTH_RATIO, windowSize.y * BUTTON_HEIGHT_RATIO))) {
		if (callback) {
			callback();
		}
	}

	ImGui::PopFont();
}

void ImGuiManager::DrawFooter(const std::string& text) {
	if (text.empty())
	{
		return;
	}
	ImVec2 windowSize = ImGui::GetWindowSize();
	ImGui::PushFont(buttonFont);
	ImGui::SetCursorPos(ImVec2(windowSize.x * 0.35f, windowSize.y * 0.85f));
	ImGui::Text("%s", text.c_str());
	ImGui::PopFont();
}

void ImGuiManager::DrawCheckbox(const std::string& label, bool* checked, float x, float y) {
	if (checked) {  // Ensure the pointer is valid before dereferencing
		ImVec2 windowSize = ImGui::GetWindowSize();

		ImGui::SetCursorPos(ImVec2(windowSize.x * x, windowSize.y * y));
		ImGui::Checkbox(label.c_str(), checked);
	}
}

