#pragma once

#include "miniaudio/miniaudio.h"

typedef struct {
  ma_sound sound;
} sound_t;

typedef struct {
  ma_decoder decoder;
  ma_sound sound;
} music_t;

void audio_init(void);
void audio_destroy(void);

void audio_sound_load(sound_t** out_sound, const char* path);
void audio_sound_play(sound_t* sound);
void audio_sound_destroy(sound_t* sound);

void audio_music_load(music_t** out_music, const char* path);
void audio_music_play(music_t* music);
void audio_music_destroy(music_t* music);
