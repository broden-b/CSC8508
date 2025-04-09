#pragma once


namespace NCL {
	namespace CSC8503 {
		class MenuState;

		class GameSettingManager {
		public:
			void SetVolume(int volume) {
				this->volume = volume;
			}
			void SetBrightness(int brightness) { this->brightness = brightness; }


			int GetVolume() const { return volume; }
			int GetBrightness() const { return brightness; }
		private:
			int volume = 50;
			int brightness;
		};
	}
}
