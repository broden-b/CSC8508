#pragma once

#include "imgui/imgui.h"

namespace NCL {
	namespace CSC8503 {
		struct PanelButton {
			std::string label;
			std::function<void()> callback;
			float xPosition;
			float yPosition;
		};

		struct PanelSlider {
			std::string label;
			int* value;
			int min;
			int max;
			float xPosition;
			float yPosition;
		};
		struct PanelCheckbox {
			std::string label;
			bool* checked;  // Store a pointer instead of a direct bool
			float xPosition;
			float yPosition;
		};



		class ImGuiManager {
		public:
			static void Initialize();
			static ImFont* GetButtonFont() { return buttonFont; }
			static ImFont* GetHeaderFont() { return headerFont; }

			static void DrawPanel(
				const std::string& title = "",
				const std::vector<PanelButton>& buttons = {},
				const std::vector<PanelSlider>& sliders = {},

				std::function<void()> backCallback = nullptr, const std::string& footer = "", const std::vector<PanelCheckbox>& checkboxes = {});

			static void DrawHeader(const std::string& title);
			static void DrawButton(const std::string& label, const std::function<void()>& callback, float verticalPos, float horizontalPos);
			static void DrawSlider(const std::string& label, int* value, int min, int max, float verticalPos, float horizontalPos);
			static void DrawBackButton(const std::function<void()>& callback);
			static void DrawFooter(const std::string& text);
			static void DrawCheckbox(const std::string& label, bool* checked, float x, float y);

			static void DrawMessagePanel(const std::string& title, const std::string& message, const ImVec4& messageColor = ImVec4(1, 1, 1, 1), std::function<void()> cancelCallback = nullptr);
			static void DrawPopupPanel(const std::string& title, const std::string& message,
				const ImVec4& messageColor, std::function<void()> acceptCallback = nullptr,
				std::function<void()> declineCallback = nullptr,
				const std::string& acceptText = "OK", const std::string& declineText = "Cancel"
			);

		private:
			static ImFont* buttonFont;
			static ImFont* headerFont;
			static ImFont* footerFont;
			static ImFont* messageFont;

			static constexpr float BUTTON_WIDTH_RATIO = 0.35f;
			static constexpr float BUTTON_HEIGHT_RATIO = 0.1f;
			static constexpr float HEADER_VERTICAL_POS = 0.1f;
			static constexpr float BACK_BUTTON_VERTICAL_POS = 0.65f;
		};
	}
}