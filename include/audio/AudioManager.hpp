#pragma once

#include "audio/Miniaudio.h"
#include "utils/NonCopyable.hpp"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <memory>
#include <filesystem>
#include <iostream>

class AudioManager : NonCopyable {
public:
	AudioManager();
	~AudioManager();
	void load(const std::string& name, const std::filesystem::path& filename, float min_distance, float max_distance, float volume);
	void loadBGM(const std::string& name, const std::filesystem::path& filename, float volume);
	bool play3D(const std::string& name, float soundX, float soundY, float soundZ);
	bool playBGM(const std::string& name, float volume);
	void stopBGM();
	void setListenerPosition(float x, float y, float z, float dirX, float dirY, float dirZ);
	void cleanFinishedSounds();

private:
	ma_engine engine;

	std::unordered_map<std::string, std::unique_ptr<ma_sound>> sound_bank;
	std::unordered_map<std::string, std::unique_ptr<ma_sound>> bgm_bank;
	std::vector<std::unique_ptr<ma_sound>> active_sounds;
	std::unique_ptr<ma_sound> current_bgm;
	//static void my_end_callback(void* pUserData, ma_sound* pSound);
};