#include <algorithm>

#include "audio/AudioManager.hpp"

AudioManager::AudioManager()
{
	ma_result init_result = ma_engine_init(nullptr, &engine);
	if (init_result != MA_SUCCESS) {
		std::cerr << "[!] FAILED TO INITIALIZE AudioManager: " << init_result << "\n";
	}
	else {
		std::cout << "Initialized Audio Manager" << std::endl;
	}
}

AudioManager::~AudioManager()
{
	ma_engine_uninit(&engine);
}

void AudioManager::load(const std::string& name, const std::filesystem::path& filename, float min_distance, float max_distance, float volume) {
	// Create sound with custom deleter
	auto new_sound = std::make_unique<ma_sound>();

	if (ma_sound_init_from_file(&engine, filename.string().c_str(), MA_SOUND_FLAG_ASYNC, nullptr, nullptr, new_sound.get()) != MA_SUCCESS) {
		std::cerr << "Failed to load sound: " << name << std::endl;
		return;
	}

	// Set some sound parameters...
	ma_sound_set_min_distance(new_sound.get(), min_distance);
	ma_sound_set_max_distance(new_sound.get(), max_distance);
	ma_sound_set_volume(new_sound.get(), volume);
	// Move the sound into the bank: owned by std::unique_ptr => MUST move (non-copyable)
	sound_bank.emplace(name, std::move(new_sound));
	std::cerr << "Loaded audio file: " << filename.string() << std::endl;
}

void AudioManager::loadBGM(const std::string& name, const std::filesystem::path& filename, float volume) {
	// Create sound with custom deleter
	auto new_sound = std::make_unique<ma_sound>();

	if (ma_sound_init_from_file(&engine, filename.string().c_str(), MA_SOUND_FLAG_ASYNC, nullptr, nullptr, new_sound.get()) != MA_SUCCESS) {
		std::cerr << "Failed to load sound: " << name << std::endl;
		return;
	}

	// Set some sound parameters
	ma_sound_set_volume(new_sound.get(), volume);
	// Move the sound into the bank: owned by std::unique_ptr => MUST move (non-copyable)
	bgm_bank.emplace(name, std::move(new_sound));
	std::cerr << "Loaded audio file: " << filename.string() << std::endl;
}

bool AudioManager::play3D(const std::string& name, float sound_x, float sound_y, float sound_z)
{
	auto it = sound_bank.find(name);
	if (it == sound_bank.end()) {
		std::cerr << "Sound not found: " << name << std::endl;
		return false;
	}

	// Create a new sound instance for playback
	auto copy_sound = std::make_unique<ma_sound>();

	// Copy the sound
	if (ma_sound_init_copy(&engine, it->second.get(), MA_SOUND_FLAG_ASYNC, nullptr, copy_sound.get()) != MA_SUCCESS) {
		std::cerr << "Failed to initialize sound copy: " << name << std::endl;
		return false;
	}

	// Set position and reset to beginning
	ma_sound_set_position(copy_sound.get(), sound_x, sound_y, sound_z);
	ma_sound_seek_to_pcm_frame(copy_sound.get(), 0);

	// Start playback
	if (ma_sound_start(copy_sound.get()) != MA_SUCCESS) {
		std::cerr << "Failed to play sound: " << name << std::endl;
		return false;
	}

	// Store in active sounds list for cleanup and cleanup finished sounds
	active_sounds.push_back(std::move(copy_sound));

	return true;
}

void AudioManager::changeVolume(double change) {
	auto volume = ma_sound_get_volume(current_bgm.get());
	volume = volume + change*0.05f;
	volume = std::clamp(volume, 0.0f, 1.0f);
	ma_sound_set_volume(current_bgm.get(), volume);
}

void AudioManager::setListenerPosition(float x, float y, float z, float dirX, float dirY, float dirZ) {
	ma_engine_listener_set_position(&engine, 0, x, y, z);
	ma_engine_listener_set_direction(&engine, 0, dirX, dirY, dirZ);
}

bool AudioManager::playBGM(const std::string& name, float volume) {
	// Stop existing BGM if it's playing
	stopBGM();

	auto it = bgm_bank.find(name);
	if (it == bgm_bank.end()) {
		std::cerr << "BGM Sound not found in bank: " << name << std::endl;
		return false;
	}

	// Create a new sound instance for playback
	current_bgm = std::make_unique<ma_sound>();

	// Copy the sound
	if (ma_sound_init_copy(&engine, it->second.get(), 0, nullptr, current_bgm.get()) != MA_SUCCESS) {
		std::cerr << "Failed to initialize BGM copy: " << name << std::endl;
		current_bgm.reset();
		return false;
	}

	// Set BGM properties
	ma_sound_set_looping(current_bgm.get(), MA_TRUE);
	ma_sound_set_volume(current_bgm.get(), volume);
	ma_sound_set_spatialization_enabled(current_bgm.get(), MA_FALSE); // 2D audio

	// Start playing
	if (ma_sound_start(current_bgm.get()) != MA_SUCCESS) {
		std::cerr << "Failed to start BGM: " << name << std::endl;
		stopBGM();
		return false;
	}

	return true;
}

void AudioManager::stopBGM() {
	if (current_bgm) {
		ma_sound_stop(current_bgm.get());
		ma_sound_uninit(current_bgm.get());
		current_bgm.reset(); // Destroys the object and sets to nullptr
	}
}

void AudioManager::cleanFinishedSounds()
{
	// Remove sounds that have finished playing
	active_sounds.erase(
		std::remove_if(active_sounds.begin(), active_sounds.end(),
			[](const std::unique_ptr<ma_sound>& sound) {
				if (!sound) return true;
				bool isPlaying = ma_sound_is_playing(sound.get());
				bool atEnd = ma_sound_at_end(sound.get());

				if (!isPlaying || atEnd) {
					ma_sound_uninit(sound.get());
					return true;
				}
				return false;
			}),
		active_sounds.end()
	);
}