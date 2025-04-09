#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include "../FMODCoreAPI/includes/fmod.hpp"

namespace NCL {
	namespace CSC8503 {
		class AudioManager {
		public:
			AudioManager();
			~AudioManager();

			// Initialize FMOD system
			bool Initialize();

			// Load a sound into the system
			void LoadSound(const std::string& filePath);
			void LoadSounds();
			void LoadSoundsXtension();
			// Play a sound
			void PlaySound(const std::string& filePath, bool loop = false);

			// Stop a sound
			void StopSound(const std::string& filePath);

			// Update the FMOD system (must be called periodically)
			void Update();
			bool IsPlaying(const std::string& filePath);
			void PrintOutputDevices();
			void SelectOutputDevice(int driverIndex);
			std::string soundFile1;
			std::string soundFile2;
			std::string soundFile3;
			std::string soundFile4;
			std::string soundFile5;
			std::string soundFile6;
			std::string soundFile7;
			std::string soundFile8;
			std::string soundFile9;
			std::string soundFile10;
			std::string soundFile11;
			std::string soundFile12;
			std::string soundFile13;
			std::string soundFile14;
			std::string soundFile15;
			std::string soundFile16;
			std::string soundFile17;
			std::string soundFile18;
			std::string soundFile19;
		private:
			FMOD::System* system;
			std::unordered_map<std::string, FMOD::Sound*> sounds;
			std::unordered_map<std::string, FMOD::Channel*> channels;

		};
	}
}
#endif // AUDIOMANAGER_H