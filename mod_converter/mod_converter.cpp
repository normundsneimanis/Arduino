//============================================================================
// MOD Converter v0.1
//----------------------------------------------------------------------------
// Spin-X Platform (http://www.spinxplatform.com)
//
// Copyright (C) 2012, Profoundic Technologies, Inc.
// http://www.profoundic.com
//----------------------------------------------------------------------------
// This file is part of Spin-X Platform (SXP). SXP is free software: you can
// redistribute it and/or modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// SXP is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along
// with SXP. If not, see http://www.gnu.org/licenses
//============================================================================

#include "sxp_src/core/main.h"
#include "sxp_src/core/fsys/fsys.h"
#include "sxp_src/core/containers.h"
using namespace pfc;
//----------------------------------------------------------------------------


//============================================================================
// config
//============================================================================
enum {modcfg_playlist_length_offset=0};
enum {modcfg_playlist_offset=2};
enum {modcfg_playlist_entry_size=2};
enum {modcfg_sample_metadata_size=8};
//----

static const char *s_copyright_message=
  "MOD Converter v0.1\r\n"
  "Copyright (C) 2013, Jarkko Lempiainen. All rights reserved.\r\n"
  "\n";
static const char *s_usage_message="Usage: mod_converter [options] -i <input.mod> -o <output.pmf>   (-h for help)\r\n";
//----------------------------------------------------------------------------


//============================================================================
// parse_command_arguments
//============================================================================
struct command_arguments
{
  command_arguments()
  {
    output_binary=false;
    suppress_copyright=false;
  }
  //----

  heap_str input_file;
  heap_str output_file;
  bool output_binary;
  bool suppress_copyright;
};
//----

bool parse_command_arguments(command_arguments &ca_, const char **args_, unsigned num_args_)
{
  // check for arguments
  if(!num_args_)
  {
    logf(s_copyright_message);
    logf(s_usage_message);
    return false;
  }

  // parse arguments
  for(unsigned i=0; i<num_args_; ++i)
  {
    // check compiler for option
    if(args_[i][0]=='-')
    {
      // switch to proper argument handling
      switch(to_lower(args_[i][1]))
      {
        // help
        case 'h':
        {
          // output help
          logf("%s"
               "%s"
               "\r\n"
               "Options:\r\n"
               "  -b            Output data as binary (instead of ASCII hex codes)\r\n"
               "  -c            Suppress copyright message\r\n", s_copyright_message, s_usage_message);
          return false;
        }

        // input file
        case 'i':
        {
          if(i<num_args_-1)
          {
            ca_.input_file=args_[i+1];
            ++i;
          }
        } break;

        // output file
        case 'o':
        {
          if(i<num_args_-1)
          {
            ca_.output_file=args_[i+1];
            ++i;
          }
        } break;

        // output binary instead of ascii hex codes
        case 'b':
        {
          ca_.output_binary=true;
        } break;

        // suppress copyright test
        case 'c':
        {
          ca_.suppress_copyright=true;
        } break;
      }
    }
  }

  return true;
}
//----------------------------------------------------------------------------


//============================================================================
// mod_sample
//============================================================================
struct mod_sample
{
  uint32 length, real_length;
  uint8 finetune;
  uint8 volume;
  uint32 loop_start, loop_len;
  owner_data data;
  unsigned sample_offset;
};
//----------------------------------------------------------------------------


//============================================================================
// mod_pattern
//============================================================================
struct mod_pattern
{
  mod_pattern() {is_used=false; pattern_offset=0;}
  //--------------------------------------------------------------------------

  array<uint8> compressed_data;
  bool is_used;
  unsigned pattern_offset;
};
//----------------------------------------------------------------------------


//============================================================================
// write_bits
//============================================================================
void write_bits(array<uint8> &comp_data_, unsigned &bit_pos_, unsigned num_bits_, uint8 v_)
{
  // write given number of bits to the arrays
  if(!bit_pos_)
    comp_data_.push_back(0);
  comp_data_.back()|=v_<<bit_pos_;
  bit_pos_+=num_bits_;
  if(bit_pos_>7)
  {
    bit_pos_-=8;
    if(bit_pos_)
    {
      comp_data_.push_back(0);
      comp_data_.back()|=v_>>(num_bits_-bit_pos_);
    }
  }
}
//----------------------------------------------------------------------------


//============================================================================
// main
//============================================================================
PFC_MAIN(const char *args_[], unsigned num_args_)
{
  // parse arguments
  command_arguments ca;
  if(!parse_command_arguments(ca, args_, num_args_))
    return 0;
  owner_ref<file_system_base> fsys=create_default_file_system(true);
  if(!ca.suppress_copyright)
    logf(s_copyright_message);

  // read signature
  owner_ref<bin_input_stream_base> in_file=open_file_read(ca.input_file.c_str());
  uint32 mod_id;
  in_file->skip(1080);
  *in_file>>mod_id;
  in_file->rewind();

  // set mod format
  unsigned num_channels=4, max_samples=15;
  switch(mod_id)
  {
    // "M.K."
    case 0x2e4b2e4d: max_samples=31; break;
    // "4CHN"
    case 0x4e484334: num_channels=4; max_samples=31; break;
    // "6CHN"
    case 0x4e484336: num_channels=6; max_samples=31; break;
    // "8CHN"
    case 0x4e484338: num_channels=8; max_samples=31; break;
    // "FLT4"
    case 0x34544c46: num_channels=4; max_samples=31; break;
    // "FLT8"
    case 0x38544c46: num_channels=4; max_samples=31; break;
  }

  // read sample infos
  array<mod_sample> samples;
  uint8 sample_indices[32]={0};
  in_file->skip(20);
  for(unsigned i=0; i<max_samples; ++i)
  {
    in_file->skip(22);
    uint16 len;
    *in_file>>len;
    len=swap_bytes(len);
    if(len>2)
    {
      mod_sample &smp=samples.push_back();
      sample_indices[i+1]=uint8(samples.size());
      uint16 loop_start, loop_len;
      *in_file>>smp.finetune>>smp.volume>>loop_start>>loop_len;
      smp.volume=smp.volume<64?smp.volume<<2:0xff;
      smp.length=unsigned(len)*2;
      smp.loop_start=unsigned(swap_bytes(loop_start))*2;
      smp.loop_len=unsigned(swap_bytes(loop_len))*2;
      if(smp.loop_len<3)
        smp.loop_len=0;
      if(smp.loop_start+smp.loop_len>smp.length)
        smp.loop_len=smp.length-smp.loop_start;
      smp.real_length=smp.loop_len?min(smp.loop_start+smp.loop_len, smp.length):smp.length;
    }
    else
      in_file->skip(6);
  }

  // read pattern playlist
  uint8 playlist_len, restart_pos;
  uint8 playlist[128];
  *in_file>>playlist_len>>restart_pos;
  in_file->read(playlist, 128);
  in_file->skip(4);

  // get number of patterns
  unsigned num_patterns=0;
  for(unsigned i=0; i<128; ++i)
    num_patterns=max(num_patterns, unsigned(playlist[i]));
  ++num_patterns;

  // read and compress patterns
  unsigned pattern_size=64*num_channels;
  array<mod_pattern> patterns(num_patterns);
  array<uint32> src_pattern_data(pattern_size);
  for(unsigned i=0; i<num_patterns; ++i)
  {
    // process pattern
    mod_pattern &pattern=patterns[i];
    in_file->read_bytes(src_pattern_data.data(), pattern_size*4);
    unsigned src_data_idx=0;
    for(unsigned ri=0; ri<64; ++ri)
    {
      // process channels in the pattern row
      unsigned channel_row_flags_idx=pattern.compressed_data.size();
      pattern.compressed_data.insert_back((num_channels+7)/8, uint8(0));
      unsigned bit_pos=0;
      uint8 flag_mask=1;
      for(unsigned ci=0; ci<num_channels; ++ci)
      {
        // update flag mask
        if(!flag_mask)
        {
          flag_mask=1;
          ++channel_row_flags_idx;
        }

        // parse channel data for the row
        uint32 chl_data=src_pattern_data[src_data_idx++];
        uint8 sample_idx=sample_indices[(chl_data&0xf0)|((chl_data>>20)&0xf)];
        uint16 note=((chl_data>>8)&0xff)|((chl_data<<8)&0xf00);
        uint8 effect=(chl_data>>16)&0xf;
        uint8 effect_data=chl_data>>24;

        // write note and sample index if defined
        if(note || sample_idx)
        {
          // find closest note
          static const int s_mod_note_periods[]=
          {
            //C     C#    D     D#    E     F     F#    G     G#    A    A#   B
               0,
            1712, 1616, 1525, 1440, 1357, 1281, 1209, 1141, 1077, 1017, 961, 907, // octave 0
             856,  808,  762,  720,  678,  640,  604,  570,  538,  508, 480, 453, // octave 1
             428,  404,  381,  360,  339,  320,  302,  285,  269,  254, 240, 226, // octave 2
             214,  202,  190,  180,  170,  160,  151,  143,  135,  127, 120, 113, // octave 4
             107,  101,   95,   90,   85,   80,   76,   71,   67,   64,  60,  57, // octave 5
          };
          unsigned min_dist=note;
          uint8 note_idx=0;
          for(unsigned i=1; i<sizeof(s_mod_note_periods)/sizeof(*s_mod_note_periods); ++i)
          {
            unsigned dist=abs(s_mod_note_periods[i]-int(note));
            if(dist<min_dist)
            {
              min_dist=dist;
              note_idx=uint8(i);
            }
          }

          // write note and sample data
          write_bits(pattern.compressed_data, bit_pos, 6, note_idx);
          write_bits(pattern.compressed_data, bit_pos, 5, sample_idx);
          pattern.compressed_data[channel_row_flags_idx]|=flag_mask;
        }
        flag_mask<<=1;

        // write effect and effect data if defined
        if(effect || effect_data)
        {
          switch(effect)
          {
            // set volume
            case 0xc: if(effect_data>63) effect_data=63; break;
          }

          write_bits(pattern.compressed_data, bit_pos, 4, effect);
          write_bits(pattern.compressed_data, bit_pos, 8, effect_data);
          pattern.compressed_data[channel_row_flags_idx]|=flag_mask;
        }
        flag_mask<<=1;
      }
    }
  }

  // setup pattern offsets
  for(unsigned i=0; i<playlist_len; ++i)
    patterns[playlist[i]].is_used=true;
  unsigned total_compressed_pattern_bytes=0;
  for(unsigned i=0; i<num_patterns; ++i)
  {
    mod_pattern &pattern=patterns[i];
    if(pattern.is_used)
    {
      pattern.pattern_offset=total_compressed_pattern_bytes;
      total_compressed_pattern_bytes+=pattern.compressed_data.size();
    }
    else
      patterns[i].compressed_data.clear();
  }
  logf("Compressed pattern data size: %i bytes (%3.1f%% of original)\r\n", total_compressed_pattern_bytes, 100.0f*float(total_compressed_pattern_bytes)/(num_patterns*pattern_size*4));

  // read sample data
  unsigned num_samples=samples.size();
  unsigned total_sample_bytes=0, total_orig_sample_bytes=0;
  for(unsigned i=0; i<num_samples; ++i)
  {
    mod_sample &smp=samples[i];
    smp.data=PFC_MEM_ALLOC(smp.length);
    in_file->read_bytes(smp.data.data, smp.length);
    smp.sample_offset=total_sample_bytes;
    total_sample_bytes+=smp.real_length;
    total_orig_sample_bytes+=smp.length;
  }
  logf("Sample data size: %i bytes (%3.1f%% of original)\r\n", total_sample_bytes, 100.0f*float(total_sample_bytes)/total_orig_sample_bytes);

  // write PMF header to a buffer
  array<uint8> pmf_data;
  container_output_stream<array<uint8> > out_stream(pmf_data);
  out_stream<<uint8(playlist_len)<<uint8(num_samples);
  unsigned pattern_offset=modcfg_playlist_offset+modcfg_playlist_entry_size*playlist_len+modcfg_sample_metadata_size*num_samples;
  for(unsigned i=0; i<playlist_len; ++i)
  {
    unsigned pattern_idx=playlist[i];
    out_stream<<uint16(pattern_offset+patterns[pattern_idx].pattern_offset);
  }
  unsigned sample_offset=pattern_offset+total_compressed_pattern_bytes;
  for(unsigned i=0; i<num_samples; ++i)
  {
    const mod_sample &smp=samples[i];
    out_stream<<uint16(sample_offset+smp.sample_offset)<<uint16(smp.real_length)<<uint16(smp.loop_len)<<uint8(smp.volume)<<uint8(smp.finetune);
  }

  // write PMF pattern and sample data
  for(unsigned i=0; i<num_patterns; ++i)
    out_stream.write_bytes(patterns[i].compressed_data.data(), patterns[i].compressed_data.size());
  for(unsigned i=0; i<num_samples; ++i)
    out_stream.write_bytes(samples[i].data.data, samples[i].real_length);
  logf("Total file data size: %i bytes\r\n", out_stream.pos());

  // write data as binary or hex codes
  owner_ref<bin_output_stream_base> out_file=open_file_write(ca.output_file.c_str());
  if(ca.output_binary)
    out_file->write_bytes(pmf_data.data(), pmf_data.size());
  else
  {
    // write data as ascii hex codes
    stack_str32 strbuf;
    unsigned data_left=pmf_data.size();
    const uint8 *bytes=pmf_data.data();
    while(data_left)
    {
      unsigned num_bytes=min<unsigned>(data_left, 256);
      for(unsigned i=0; i<num_bytes; ++i)
      {
        strbuf.format("0x%02x, ", bytes[i]);
        *out_file<<strbuf.c_str();
      }
      *out_file<<"\r\n";
      bytes+=num_bytes;
      data_left-=num_bytes;
    }
  }

  return 0;
}
//----------------------------------------------------------------------------
