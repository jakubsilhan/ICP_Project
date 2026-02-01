#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <filesystem>
#include <iostream>

#include "audio/Miniaudio.h"
#include "utils/NonCopyable.hpp"

class AudioManager : NonCopyable {
public:
	AudioManager();
	~AudioManager();
	void load(const std::string& name, const std::filesystem::path& filename, float min_distance, float max_distance, float volume);
	void load_BGM(const std::string& name, const std::filesystem::path& filename, float volume);
	bool play_3D(const std::string& name, float sound_x, float sound_y, float sound_z);
	bool play_BGM(const std::string& name, float volume);
	void stop_BGM();
	void change_volume(double change);
	void set_listener_position(float x, float y, float z, float dirX, float dirY, float dirZ);
	void clean_finished_sounds();

private:
	ma_engine engine;

	std::unordered_map<std::string, std::unique_ptr<ma_sound>> sound_bank;
	std::unordered_map<std::string, std::unique_ptr<ma_sound>> bgm_bank;
	std::vector<std::unique_ptr<ma_sound>> active_sounds;
	std::unique_ptr<ma_sound> current_bgm;
};
