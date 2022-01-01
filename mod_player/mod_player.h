//===========================================================================
// Mod Player v0.1
//---------------------------------------------------------------------------
// Copyright (c) 2013 Jarkko Lempiainen
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
//   1. The origin of this software must not be misrepresented; you must not
//   claim that you wrote the original software. If you use this software
//   in a product, an acknowledgment in the product documentation would be
//   appreciated but is not required.
//
//   2. Altered source versions must be plainly marked as such, and must not
//   be misrepresented as being the original software.
// 
//   3. This notice may not be removed or altered from any source
//   distribution.
//===========================================================================

#ifndef PFC_MOD_PLAYER_H
#define PFC_MOD_PLAYER_H
//---------------------------------------------------------------------------


//============================================================================
// interface
//============================================================================
// external
#include <Arduino.h>

// new
class mod_player;
//---------------------------------------------------------------------------


//===========================================================================
// MOD player config
//===========================================================================
enum {modplayer_sampling_rate=20000};
enum {modplayer_max_channels=4};
enum {modplayer_led_beat_ticks=3};
//---------------------------------------------------------------------------


//===========================================================================
// mod_player
//===========================================================================
class mod_player
{
public:
  // construction
  mod_player();
  ~mod_player();
  //-------------------------------------------------------------------------

  // player control
  void start(const void *pmem_mod_file_);
  void stop();
  void update();
  //-------------------------------------------------------------------------

private:
  static void timer_interrupt();
  void mix_buffer_batch();
  void apply_channel_effects();
  void process_pattern_row();
  //-------------------------------------------------------------------------

  //=========================================================================
  // audio_channel
  //=========================================================================
  struct audio_channel
  {
    uint32_t sample_pos;   // sample position (24.8 fp)
    const int8_t *sample;  // channel sample (in program memory)
    uint16_t sample_speed; // sample speed (8.8 fp)
    uint16_t sample_end;   // sample end position (16.0 fp)
    uint16_t loop_len;     // sample loop length (16.0 fp)
    uint16_t note_period;  // current note period (see s_mod_note_periods for base note periods)
    uint8_t base_note_idx; // base note index
    uint8_t volume;        // sample volume (0.8 fp)
    uint8_t effect;        // current effect
    uint8_t effect_data;   // current effect data
    uint8_t led_beat_cnt;  // LED beat counter
  };
  //-------------------------------------------------------------------------

  enum {buffer_batch_size=modplayer_sampling_rate/50};
  enum {buffer_size=buffer_batch_size*2};
  static mod_player *s_active;
  // audio assets
  const uint8_t *m_mod_file;
  // audio channel states
  audio_channel m_channels[modplayer_max_channels];
  // audio buffer state
  uint8_t m_buffer[buffer_size];
  volatile uint16_t m_buffer_playback_pos;
  uint8_t m_buffer_batch_write_idx;
  // pattern playback state
  uint8_t m_current_pattern_playlist_pos;
  uint8_t m_current_pattern_row;
  uint8_t m_current_row_tick;
  uint8_t m_arpeggio_counter;
  uint8_t m_num_ticks_per_row;
  const uint8_t *m_active_pattern_row;
};
//---------------------------------------------------------------------------

//============================================================================
#endif
