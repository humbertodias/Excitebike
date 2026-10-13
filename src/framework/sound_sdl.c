#include <SDL.h>
#include <SDL_mixer.h>
#include <stdlib.h>
#include <string.h>

#include "sound.h"

typedef struct sound_source_t {
	const char* filename;
	Mix_Chunk* chunk;
	int channel;
} SoundSource;

static struct sound_manager_t {
	SoundSource* sounds;
	int32_t maxSounds;
	bool open;
} _soundMgr = { NULL, 0, false };

bool soundInit(int32_t maxSounds)
{
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
		return false;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
		return false;
	}

	Mix_AllocateChannels(maxSounds);
	_soundMgr.sounds = calloc((size_t)maxSounds, sizeof(SoundSource));
	if (_soundMgr.sounds == NULL) {
		return false;
	}
	for (int32_t i = 0; i < maxSounds; ++i) {
		_soundMgr.sounds[i].channel = -1;
	}
	_soundMgr.maxSounds = maxSounds;
	_soundMgr.open = true;
	return true;
}

bool soundShutdown(void)
{
	if (_soundMgr.sounds != NULL) {
		for (int32_t i = 0; i < _soundMgr.maxSounds; ++i) {
			soundUnload(i);
		}
		free(_soundMgr.sounds);
		_soundMgr.sounds = NULL;
	}
	if (_soundMgr.open) {
		Mix_CloseAudio();
		_soundMgr.open = false;
	}
	return true;
}

int32_t soundLoad(const char* filename)
{
	for (int32_t i = 0; i < _soundMgr.maxSounds; ++i) {
		if (_soundMgr.sounds[i].filename == NULL) {
			Mix_Chunk* chunk = Mix_LoadWAV(filename);
			if (chunk == NULL) {
				return SOUND_NOSOUND;
			}
			_soundMgr.sounds[i].filename = filename;
			_soundMgr.sounds[i].chunk = chunk;
			_soundMgr.sounds[i].channel = -1;
			return i;
		}
	}
	return SOUND_NOSOUND;
}

void soundUnload(int32_t soundId)
{
	if (soundId == SOUND_NOSOUND || _soundMgr.sounds == NULL) {
		return;
	}

	SoundSource* sound = &_soundMgr.sounds[soundId];
	if (sound->channel >= 0) {
		Mix_HaltChannel(sound->channel);
		sound->channel = -1;
	}
	if (sound->chunk != NULL) {
		Mix_FreeChunk(sound->chunk);
		sound->chunk = NULL;
	}
	sound->filename = NULL;
}

void soundPlay(int32_t soundId)
{
	if (soundId == SOUND_NOSOUND || _soundMgr.sounds == NULL) {
		return;
	}

	SoundSource* sound = &_soundMgr.sounds[soundId];
	if (sound->chunk == NULL) {
		return;
	}
	sound->channel = Mix_PlayChannel(-1, sound->chunk, 0);
}

void soundStop(int32_t soundId)
{
	if (soundId == SOUND_NOSOUND || _soundMgr.sounds == NULL) {
		return;
	}

	SoundSource* sound = &_soundMgr.sounds[soundId];
	if (sound->channel >= 0) {
		Mix_HaltChannel(sound->channel);
		sound->channel = -1;
	}
}
