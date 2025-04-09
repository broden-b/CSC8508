#pragma once

#include <fmod_errors.h>  // For FMOD error strings
#include <iostream>
#include <string>
#include <unordered_map>

#include "PrisonEscape/Core/AudioManager.h"
#include "PrisonEscape/Core/GameBase.h"
#include "PrisonEscape/Core/GameSettingManager.h"

using namespace NCL;
using namespace CSC8503;

AudioManager::AudioManager()
{

}

AudioManager::~AudioManager() {
	// Release all sounds and the FMOD system when the AudioManager is destroyed
	for (auto& soundPair : sounds) {
		soundPair.second->release();
	}
	system->close();
	system->release();
}

bool AudioManager::Initialize() {
	FMOD_RESULT result = FMOD::System_Create(&system);
	if (result != FMOD_OK) {
		std::cerr << "Failed to create FMOD system!" << std::endl;
		return false;
	}

	result = system->init(512, FMOD_INIT_NORMAL, nullptr);
	if (result != FMOD_OK) {
		std::cerr << "Failed to initialize FMOD!" << std::endl;
		return false;
	}

	return true;
}

void AudioManager::LoadSound(const std::string& filePath) {
	if (sounds.find(filePath) != sounds.end()) {
		std::cerr << "Sound already loaded: " << filePath << std::endl;
		return;
	}

	FMOD::Sound* sound = nullptr;
	FMOD_RESULT result = system->createSound(filePath.c_str(), FMOD_DEFAULT, nullptr, &sound);
	if (result != FMOD_OK) {
		std::cerr << "Failed to load sound: " << filePath << " Error: " << FMOD_ErrorString(result) << std::endl;
		return;
	}

	sounds[filePath] = sound;
	std::cout << "Sound loaded: " << filePath << std::endl;
}

void AudioManager::PlaySound(const std::string& filePath, bool loop) {
	// Check if the sound is loaded
	if (sounds.find(filePath) == sounds.end()) {
		std::cerr << "Sound not loaded: " << filePath << std::endl;
		return;
	}

	FMOD::Sound* sound = sounds[filePath];
	FMOD::Channel* channel = nullptr;

	std::cout << "Attempting to play sound: " << filePath << std::endl;
	bool isMuted = false;
	FMOD_RESULT result = system->playSound(sound, nullptr, isMuted, &channel);
	if (result != FMOD_OK) {
		std::cerr << "Failed to play sound: " << FMOD_ErrorString(result) << std::endl;
		return;
	}

	FMOD_RESULT result2 = channel->getMute(&isMuted);

	if (result != FMOD_OK) {
		std::cerr << "Error checking mute status: " << FMOD_ErrorString(result) << std::endl;
	}
	else {
		if (isMuted) {
			std::cout << "The sound is muted." << std::endl;
		}
		else {
			std::cout << "The sound is not muted." << std::endl;
		}
	}
	// Set looping if requested
	result = channel->setLoopCount(loop ? -1 : 0);
	if (result != FMOD_OK) {
		std::cerr << "Failed to set loop count: " << FMOD_ErrorString(result) << std::endl;
		return;
	}

	// Set volume for debugging to make sure the sound isn't muted
	result = channel->setVolume(1.0f);  // Full volume
	if (result != FMOD_OK) {
		std::cerr << "Failed to set volume: " << FMOD_ErrorString(result) << std::endl;
	}

	// Output the status of the channel (whether it is playing or not)
	bool isPlaying = false;
	result = channel->isPlaying(&isPlaying);
	if (result != FMOD_OK) {
		std::cerr << "Error checking if sound is playing: " << FMOD_ErrorString(result) << std::endl;
	}
	else {
		if (isPlaying) {
			std::cout << "Sound is playing!" << std::endl;
		}
		else {
			std::cerr << "Sound is not playing." << std::endl;
		}
	}

	channels[filePath] = channel;

}

void AudioManager::StopSound(const std::string& filePath) {
	// Check if the sound is playing
	if (channels.find(filePath) == channels.end()) {
		std::cerr << "Sound is not playing: " << filePath << std::endl;
		return;
	}

	FMOD::Channel* channel = channels[filePath];
	channel->stop();
	channels.erase(filePath);
	std::cout << "Stopped sound: " << filePath << std::endl;
}

void AudioManager::Update() {
	// Update the FMOD system
	FMOD_RESULT result = system->update();
	if (result != FMOD_OK) {
		std::cerr << "FMOD update failed: " << FMOD_ErrorString(result) << std::endl;
	}

	/*// Check if any of the sounds are still playing
	for (auto& channelPair : channels) {
		FMOD::Channel* channel = channelPair.second;
		bool isPlaying = false;

		// Check if the sound is playing
		result = channel->isPlaying(&isPlaying);
		if (result != FMOD_OK) {
			std::cerr << "Error checking if sound is playing: " << FMOD_ErrorString(result) << std::endl;
		}
		else if (isPlaying) {
			std::cout << "Sound '" << channelPair.first << "' is playing!" << std::endl;
		}
		else {
			std::cout << "Sound '" << channelPair.first << "' is not playing." << std::endl;
		}
	}*/
	if (GameBase::GetGameSettings()) {
		float volume = GameBase::GetGameSettings()->GetVolume() / 100.0f; // Convert to 0.0 - 1.0 range
		for (auto& pair : channels) {
			FMOD::Channel* channel = pair.second;
			if (channel) {
				channel->setVolume(volume);
			}
		}
	}
	system->update();
}
void AudioManager::PrintOutputDevices() {
	int numDrivers = 0;
	FMOD_RESULT result = system->getNumDrivers(&numDrivers);
	if (result != FMOD_OK) {
		std::cerr << "Error getting number of drivers: " << FMOD_ErrorString(result) << std::endl;
		return;
	}

	if (numDrivers == 0) {
		std::cerr << "No audio drivers found." << std::endl;
		return;
	}

	std::cout << "Available output devices:" << std::endl;
	for (int i = 0; i < numDrivers; ++i) {
		char name[256];
		result = system->getDriverInfo(i, name, sizeof(name), nullptr, nullptr, nullptr, nullptr);
		if (result == FMOD_OK) {
			std::cout << "Driver " << i << ": " << name << std::endl;
		}
		else {
			std::cerr << "Error getting driver info: " << FMOD_ErrorString(result) << std::endl;
		}
	}

	// Get the current driver
	int currentDriver = 0;
	result = system->getDriver(&currentDriver);
	if (result == FMOD_OK) {
		std::cout << "Currently using driver " << currentDriver << std::endl;
	}
	else {
		std::cerr << "Error getting current driver: " << FMOD_ErrorString(result) << std::endl;
	}

}

void AudioManager::SelectOutputDevice(int driverIndex) {
	FMOD_RESULT result = system->setDriver(driverIndex);
	if (result != FMOD_OK) {
		std::cerr << "Failed to set output device: " << FMOD_ErrorString(result) << std::endl;
	}
	else {
		std::cout << "Output device set to driver " << driverIndex << std::endl;
	}
}
void AudioManager::LoadSounds() {
	soundFile9 = "PrisonEscape/Assets/SFX/tension-suspense.mp3";
	LoadSound(soundFile9);
}
void AudioManager::LoadSoundsXtension() {
	soundFile1 = "PrisonEscape/Assets/SFX/Shotgun.wav";
	soundFile2 = "PrisonEscape/Assets/SFX/button.mp3";
	soundFile3 = "PrisonEscape/Assets/SFX/coin_toss.mp3";
	soundFile4 = "PrisonEscape/Assets/SFX/grunt-1.mp3";
	soundFile5 = "PrisonEscape/Assets/SFX/old-radio.mp3";
	soundFile6 = "PrisonEscape/Assets/SFX/opening-metal.mp3";
	soundFile7 = "PrisonEscape/Assets/SFX/Prison-Escape-v1.mp3";
	soundFile8 = "PrisonEscape/Assets/SFX/walking-on-wooden-floor.mp3";
	soundFile10 = "PrisonEscape/Assets/SFX/grunt-2.mp3";
	soundFile11 = "PrisonEscape/Assets/SFX/heavy-breathing.mp3";
	soundFile12 = "PrisonEscape/Assets/SFX/soap-bubbles-pop-96873.mp3";

	LoadSound(soundFile1);
	LoadSound(soundFile2);
	LoadSound(soundFile3);
	LoadSound(soundFile4);
	LoadSound(soundFile5);
	LoadSound(soundFile6);
	LoadSound(soundFile7);
	LoadSound(soundFile8);
	LoadSound(soundFile10);
	LoadSound(soundFile11);
	LoadSound(soundFile12);
	
}
bool AudioManager::IsPlaying(const std::string& filePath) {
	auto it = channels.find(filePath);
	if (it != channels.end() && it->second) {
		bool isPlaying = false;
		it->second->isPlaying(&isPlaying);
		return isPlaying;
	}
	return false;
}
