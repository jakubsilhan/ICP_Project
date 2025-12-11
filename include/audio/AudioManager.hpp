#include "audio/Miniaudio.h"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <memory>
#include <filesystem>
#include <iostream>

class AudioManager {
public:
	void load(const std::string& name, const std::filesystem::path& filename);
	bool play3D(const std::string& name, float soundX, float soundY, float soundZ, float listX, float listY, float listZ, float listXDir, float listYDir, float listZDir);

private:
	ma_engine engine;

	std::unordered_map<std::string, std::unique_ptr<ma_sound, void(*)(ma_sound*)>> sound_bank;
	std::unordered_set<ma_sound*> active_sounds;
	static void my_end_callback(void* pUserData, ma_sound* pSound);
	void setListenerPosition(float x, float y, float z, float dirX, float dirY, float dirZ);
};