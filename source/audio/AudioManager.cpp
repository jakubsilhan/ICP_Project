#include "audio/AudioManager.hpp"

AudioManager::AudioManager() {
	// This function MUST be called to set up the ma_engine internal pointers
	ma_result result = ma_engine_init(nullptr, &engine);

	if (result != MA_SUCCESS) {
		std::cerr << "CRITICAL: Failed to initialize miniaudio engine! Error code: " << result << std::endl;
	}
	else {
		std::cout << "Audio Engine initialized successfully." << std::endl;
	}
}

AudioManager::~AudioManager() {
	// Clean up the engine when the manager is destroyed
	ma_engine_uninit(&engine);
}

void AudioManager::load(const std::string& name, const std::filesystem::path& filename) {
	// sound with custom deleter (function, that deactivates and deallocates sound)
	std::unique_ptr<ma_sound, void(*)(ma_sound*)> new_snd(new ma_sound{}, [](ma_sound* pSnd) { ma_sound_uninit(pSnd); delete pSnd; });
	if (ma_sound_init_from_file(&engine, filename.string().c_str(), 0, nullptr, nullptr, new_snd.get()) != MA_SUCCESS) {
		std::cerr << "Failed to load sound:" << name << std::endl;
		//delete new_snd;
	}
	else {
		// set some sound parameters...
		ma_sound_set_min_distance(new_snd.get(), 0.5f);
		ma_sound_set_max_distance(new_snd.get(), 100.0f);
		ma_sound_set_volume(new_snd.get(), 2.0f);
		// move the sound into the bank: owned by std::unique_ptr => MUST move (non-copyable)
		sound_bank.emplace(name, std::move(new_snd));
	}
}

bool AudioManager::play3D(const std::string& name, float soundX, float soundY, float soundZ, float listX, float listY, float listZ, float
	listXDir, float listYDir, float listZDir) {
	ma_sound* original = sound_bank.at(name).get(); // get raw pointer from smart-pointer
	ma_sound* copy_snd = new ma_sound; // will be dealloc. by callback
	if (ma_sound_init_copy(&engine, original, 0, nullptr, copy_snd) != MA_SUCCESS) {
		std::cerr << "Failed to copy sound: " << name << std::endl;
		delete copy_snd;
		return false;
	}

	ma_sound_set_end_callback(copy_snd, my_end_callback, this); // set callback for automated deletion
	ma_sound_seek_to_pcm_frame(copy_snd, 0); // reset to beginning
	ma_sound_set_position(copy_snd, soundX, soundY, soundZ); // set the sound properties
	setListenerPosition(listX, listY, listZ, listXDir, listYDir, listZDir); // set listener position
	// play the sound
	if (ma_sound_start(copy_snd) != MA_SUCCESS) {
		std::cerr << "Failed to play sound: " << name << std::endl;
		return false;
	}
	// remember as active; will be automatically cleared from active when callback fires
	active_sounds.insert(copy_snd);
	return true;
}

void AudioManager::setListenerPosition(float x, float y, float z, float dirX, float dirY, float dirZ) {
	ma_engine_listener_set_position(&engine, 0, x, y, z);
	ma_engine_listener_set_direction(&engine, 0, dirX, dirY, dirZ);
}

// TODO check this
void AudioManager::my_end_callback(void* pUserData, ma_sound* pSound) {
	auto t = static_cast<AudioManager*>(pUserData); // get current instance
	if (!ma_sound_is_playing(pSound)) {
		t->active_sounds.erase(pSound);
		ma_sound_uninit(pSound);
		delete pSound;
	}
}