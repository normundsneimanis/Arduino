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

#include "mod_player.h"
#include <TimerOne.h>
//---------------------------------------------------------------------------


//===========================================================================
// mod format config
//===========================================================================
enum {modcfg_playlist_length_offset=0};
enum {modcfg_playlist_offset=2};
enum {modcfg_playlist_entry_size=2};
enum {modcfg_sample_metadata_size=8};
//---------------------------------------------------------------------------


//===========================================================================
// MOD note periods
//===========================================================================
static const uint16_t s_mod_note_periods[]=
{
//  C     C#    D     D#    E     F     F#    G     G#    A    A#   B
  1712, 1616, 1525, 1440, 1357, 1281, 1209, 1141, 1077, 1017, 961, 907, // octave 0
   856,  808,  762,  720,  678,  640,  604,  570,  538,  508, 480, 453, // octave 1
   428,  404,  381,  360,  339,  320,  302,  285,  269,  254, 240, 226, // octave 2
   214,  202,  190,  180,  170,  160,  151,  143,  135,  127, 120, 113, // octave 4
   107,  101,   95,   90,   85,   80,   76,   71,   67,   64,  60,  57, // octave 5
};
//---------------------------------------------------------------------------


//===========================================================================
// local helper functions
//===========================================================================
namespace
{
  //=========================================================================
  // read bits
  //=========================================================================
  uint8_t read_bits(const uint8_t *&ptr_, uint8_t &bit_pos_, uint8_t num_bits_)
  {
    uint8_t v=pgm_read_word(ptr_)>>bit_pos_;
    bit_pos_+=num_bits_;
    if(bit_pos_>7)
    {
      ++ptr_;
      bit_pos_-=8;
    }
    return v;
  }
  //-------------------------------------------------------------------------

  //=========================================================================
  // get_sample_speed
  //=========================================================================
  uint16_t get_sample_speed(uint16_t note_period_)
  {
    return uint16_t((256.0f*7093789.2/modplayer_sampling_rate)/note_period_);
  }
} // namespace <anonymous>
//---------------------------------------------------------------------------


//===========================================================================
// mod_player
//===========================================================================
mod_player *mod_player::s_active=0;
//---------------------------------------------------------------------------

mod_player::mod_player()
{
  // initialize members
  memset(m_buffer, 0x80, sizeof(m_buffer));
  m_buffer_playback_pos=0;
  m_buffer_batch_write_idx=0;
  memset(m_channels, 0, sizeof(m_channels));
}
//----

mod_player::~mod_player()
{
  stop();
}
//---------------------------------------------------------------------------

void mod_player::start(const void *pmem_mod_file_)
{
  // activate player and setup pins
  if(s_active)
    return;
  s_active=this;
  DDRD=0xff;
  DDRB=0x0f;

  // init playback
  m_mod_file=static_cast<const uint8_t*>(pmem_mod_file_);
  m_current_pattern_playlist_pos=0;
  m_current_pattern_row=0;
  m_num_ticks_per_row=6;
  m_current_row_tick=m_num_ticks_per_row-1;
  m_arpeggio_counter=0;
  m_active_pattern_row=m_mod_file+pgm_read_word(m_mod_file+modcfg_playlist_offset);

  // start buffer playback
  Timer1.attachInterrupt(timer_interrupt);
  Timer1.initialize(1000000/modplayer_sampling_rate);
}
//----

void mod_player::stop()
{
  // stop playback
  if(!s_active)
    return;
  Timer1.stop();
  Timer1.detachInterrupt();
  s_active=0;
}
//----

void mod_player::update()
{
  // check for audio channel and buffer update
  if(   (m_buffer_batch_write_idx && m_buffer_playback_pos<buffer_batch_size)
     || (!m_buffer_batch_write_idx && m_buffer_playback_pos>=buffer_batch_size))
  {
    if(++m_current_row_tick==m_num_ticks_per_row)
    {
      process_pattern_row();
      m_current_row_tick=0;
    }
    apply_channel_effects();
    mix_buffer_batch();
  }
}
//---------------------------------------------------------------------------

void mod_player::timer_interrupt()
{
  PORTD=s_active->m_buffer[s_active->m_buffer_playback_pos];
  if(++s_active->m_buffer_playback_pos==buffer_size)
    s_active->m_buffer_playback_pos=0;
}
//----

void mod_player::mix_buffer_batch()
{
  // mix batch of samples
  uint8_t *buf=m_buffer+(m_buffer_batch_write_idx?buffer_batch_size:0);
  m_buffer_batch_write_idx^=1;
  for(unsigned si=0; si<buffer_batch_size; ++si)
  {
    // mix sample and advance all channels
    int16_t res=0x80*modplayer_max_channels;
    for(uint8_t ci=0; ci<modplayer_max_channels; ++ci)
    {
      // mix channel sample and advance sample position
      audio_channel &chl=m_channels[ci];
      int8_t smp=(int8_t)pgm_read_byte(chl.sample+(chl.sample_pos>>8));
      res+=(smp*chl.volume)>>8;
      chl.sample_pos+=chl.sample_speed;
      if((chl.sample_pos>>8)>=chl.sample_end)
      {
        chl.sample_pos-=long(chl.loop_len)<<8;
        if(!chl.loop_len)
        {
          chl.sample_pos=0;
          chl.sample_speed=0;
          chl.volume=0;
        }
      }
    }

    // clip sample and write it to the buffer
    res/=modplayer_max_channels;
    *buf++=res<0?0:res>0xff?0xff:res;
  }
}
//----

void mod_player::apply_channel_effects()
{
  m_arpeggio_counter=(m_arpeggio_counter+1)%3;
  uint8_t led_bits=0;
  for(unsigned ci=0; ci<modplayer_max_channels; ++ci)
  {
    audio_channel &chl=m_channels[ci];
    if(chl.led_beat_cnt)
    {
      --chl.led_beat_cnt;
      led_bits|=1<<ci;
    }
    else
    switch(chl.effect)
    {
      // arpeggio
      case 0x0:
      {
        chl.note_period=s_mod_note_periods[chl.base_note_idx+((chl.effect_data>>(4*m_arpeggio_counter))&0xf)];
        chl.sample_speed=get_sample_speed(chl.note_period);
      } break;

      // slide up
      case 0x1:
      {
        chl.note_period-=chl.effect_data;
        if(int(chl.note_period)<57)
          chl.note_period=57;
        chl.sample_speed=get_sample_speed(chl.note_period);
      } break;

      // slide down
      case 0x2:
      {
        chl.note_period+=chl.effect_data;
        if(chl.note_period>1712)
          chl.note_period=1712;
        chl.sample_speed=get_sample_speed(chl.note_period);
      } break;

      // volume slide
      case 0xa:
      {
        int8_t vdelta=((chl.effect_data>>4)-(chl.effect_data&0xf))<<2;
        int16_t v=int16_t(chl.volume)+vdelta;
        chl.volume=v<0?0:v>255?255:v;
      } break;
    }
  }
  PORTB=led_bits;
}
//----

void mod_player::process_pattern_row()
{
  // parse row in the music pattern
  const uint8_t *chl_flags_ptr=m_active_pattern_row;
  m_active_pattern_row+=(modplayer_max_channels*2+7)/8;
  uint8_t bit_pos=0;
  uint8_t chl_flags=0, chl_flag_mask=0;
  for(uint8_t ci=0; ci<modplayer_max_channels; ++ci)
  {
    // get channel flags
    if(!chl_flag_mask)
    {
      chl_flag_mask=1;
      chl_flags=pgm_read_byte(chl_flags_ptr++);
    }

    // get note and instrument
    audio_channel &chl=m_channels[ci];
    if(chl_flags&chl_flag_mask)
    {
      uint8_t note=read_bits(m_active_pattern_row, bit_pos, 6)&0x3f;
      if(note)
      {
        uint8_t note_idx=note-1;
        chl.note_period=s_mod_note_periods[note_idx];
        chl.base_note_idx=note_idx;
        chl.sample_pos=0;
        chl.sample_speed=get_sample_speed(chl.note_period);
        chl.led_beat_cnt=modplayer_led_beat_ticks;
      }
      uint8_t smp_idx=read_bits(m_active_pattern_row, bit_pos, 5)&0x1f;
      if(smp_idx)
      {
        const uint8_t *smp_metadata=m_mod_file+modcfg_playlist_offset+pgm_read_byte(m_mod_file+modcfg_playlist_length_offset)*modcfg_playlist_entry_size+(smp_idx-1)*modcfg_sample_metadata_size;
        chl.sample=(const int8_t*)m_mod_file+pgm_read_word(smp_metadata);
        chl.sample_end=pgm_read_word(smp_metadata+2);
        chl.loop_len=pgm_read_word(smp_metadata+4);
        chl.volume=pgm_read_byte(smp_metadata+8);
      }
    }
    chl_flag_mask<<=1;

    // get effect
    if(chl_flags&chl_flag_mask)
    {
      uint8_t effect=read_bits(m_active_pattern_row, bit_pos, 4)&0xf;
      uint8_t effect_data=read_bits(m_active_pattern_row, bit_pos, 8);
      switch(effect)
      {
        // position jump
        case 0xb:
        {
          m_current_pattern_playlist_pos=effect_data-1;
          m_current_pattern_row=63;
        } break;

        // set volume
        case 0xc:
        {
          chl.volume=effect_data<64?effect_data<<2:0xff;
        } break;

        // pattern break
        case 0xd:
        {
          /*todo: add support for row position*/
          m_current_pattern_row=63;
        } break;

        // set tempo
        case 0xf:
        {
          if(effect_data<33)
            m_num_ticks_per_row=effect_data;
          else
          {
            /*todo: add support for setting BPM*/
          }
        } break;
      }

      chl.effect=effect;
      chl.effect_data=effect_data;
    }
    chl_flag_mask<<=1;
  }

  // advance pattern
  if(bit_pos)
    ++m_active_pattern_row;
  if(++m_current_pattern_row==64)
  {
    // proceed to the next pattern
    if(++m_current_pattern_playlist_pos==pgm_read_byte(m_mod_file+modcfg_playlist_length_offset))
      m_current_pattern_playlist_pos=0;
    m_active_pattern_row=m_mod_file+pgm_read_word(m_mod_file+modcfg_playlist_offset+m_current_pattern_playlist_pos*2);
    m_current_pattern_row=0;
  }
}
//---------------------------------------------------------------------------
