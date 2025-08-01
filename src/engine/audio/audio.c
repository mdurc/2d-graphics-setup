#include "audio.h"

#include "../c-lib/log.h"
#include "../c-lib/misc.h"

#define MA_IMPLEMENTATION
#include "miniaudio/miniaudio.h"

ma_engine engine;

void audio_init(void) {
  if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
    ERROR_EXIT("miniaudio failed init");
  }
  ma_engine_set_volume(&engine, 0.1f);
  LOG("Audio system initialized");
}

void audio_destroy(void) {
  ma_engine_uninit(&engine);
  LOG("Audio system deinitialized");
}

void audio_sound_load(sound_t** out_sound, const char* path) {
  *out_sound = (sound_t*)malloc(sizeof(sound_t));
  if (ma_sound_init_from_file(&engine, path, 0, NULL, NULL,
                              &(*out_sound)->sound) != MA_SUCCESS) {
    ERROR_EXIT("failed to load sound audio file: %s", path);
  }
  LOG("Successfully loaded audio sound file: %s", path);
}

void audio_sound_play(sound_t* sound) { ma_sound_start(&sound->sound); }

void audio_sound_destroy(sound_t* sound) {
  if (sound) {
    ma_sound_uninit(&sound->sound);
    free(sound);
  }
}

void audio_music_load(music_t** out_music, const char* path) {
  *out_music = (music_t*)malloc(sizeof(music_t));
  if (ma_decoder_init_file(path, NULL, &(*out_music)->decoder) != MA_SUCCESS) {
    ERROR_EXIT("failed to load music audio file: %s\n", path);
  }

  if (ma_sound_init_from_data_source(&engine, &(*out_music)->decoder,
                                     MA_SOUND_FLAG_STREAM, NULL,
                                     &(*out_music)->sound) != MA_SUCCESS) {
    ERROR_EXIT("failed to create streaming sound for music");
  }
  LOG("Successfully loaded audio music file: %s", path);
}

void audio_music_play(music_t* music) {
  ma_sound_set_looping(&music->sound, MA_TRUE);
  ma_sound_start(&music->sound);
}

void audio_music_destroy(music_t* music) {
  if (music) {
    ma_sound_uninit(&music->sound);
    ma_decoder_uninit(&music->decoder);
    free(music);
  }
}
