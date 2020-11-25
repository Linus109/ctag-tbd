/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de
(c) 2020 by Robert Manzke. All rights reserved.

(c) 2020 by Mathias Brüssel for "ByteBeatsXFade"-Plugin. All rights reserved.
Includes ByteBeat algorithms by Matt Wand - make sure to have a look at his other great work at: hot-air.bandcamp.com

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "ctagSoundProcessorbbeats.hpp"

using namespace CTAG::SP;

// data.buf[i * 2 + processCh] = (float)((byte)(t & 128));    // Square wave, working ok!

// --- Helper function: rescale CV or Pot to integer of given range 0...max ---
inline int ctagSoundProcessorbbeats::process_param( const ProcessData &data, int cv_myparm, int my_parm, int parm_range, int max_idx )
{
  if (cv_myparm != -1)
  {
    if( data.cv[cv_myparm] >= 0.0 )       // This is a bypass solution to avoid negative values in rare cases
      return (int)(data.cv[cv_myparm] * parm_range);
    else
      return 0;
  }
  else
    return my_parm * parm_range / max_idx;
}

// --- Helper function: rescale CV or Pot to float 0...1.0 (CV is already in correct format, we still keep it inside this method for convenience ---
inline float ctagSoundProcessorbbeats::process_param_float( const ProcessData &data, int cv_myparm, int my_parm, int max_idx )
{
  // printf("%d, %d \n", cv_myparm, my_parm);
  if(cv_myparm != -1)
  {
    if (data.cv[cv_myparm] >= 0.0f)     // This is a bypass solution to avoid negative values in rare cases
      return data.cv[cv_myparm];
    else
      return 0.0f;
  }
  else
    return my_parm / (float)max_idx;
}


inline bool ctagSoundProcessorbbeats::process_param_bool( const ProcessData &data, int cv_myparm, int my_parm )
{
  if (cv_myparm != -1)
  {
    if (data.cv[cv_myparm] >= 0.0f)     // This is a bypass solution to avoid negative values in rare cases
      return true;
    else
      return false;
  }
  else
    return( (bool) my_parm );
}

// --- Helper function: provide logic operations on bytebeats ---
inline float ctagSoundProcessorbbeats::logic_operation_on_beat()
{
  switch( logic_operation_id )
  {
    case 0:   // Or operation
      return ((beat_byte_A | beat_byte_B)-127)/127.0f;

    case 1:   // NOR A operation
      return ((~beat_byte_A | beat_byte_B)-127)/127.0f;

    case 2:   // NOR B operation
      return ((beat_byte_A | ~beat_byte_B)-127)/127.0f;

    case 3:   // NOR A-B operation
      return ((~beat_byte_A | ~beat_byte_B)-127)/127.0f;


    case 4:   // And operation
      return ((beat_byte_A & beat_byte_B)-127)/127.0f;

    case 5:   // NAND A operation
      return ((~beat_byte_A & beat_byte_B)-127)/127.0f;

    case 6:   // NAND B operation
      return ((beat_byte_A & ~beat_byte_B)-127)/127.0f;

    case 7:   // NAND A-B operation
      return ((~beat_byte_A & ~beat_byte_B)-127)/127.0f;


    case 8:   // XOR operation
      return ((beat_byte_A ^ beat_byte_B)-127)/127.0f;

    case 9:   // N-XOR  A operation
      return ((~beat_byte_A ^ beat_byte_B)-127)/127.0f;

    case 10:   // N-XOR B operation
      return ((beat_byte_A ^ ~beat_byte_B)-127)/127.0f;

    case 11:   // N-XOR A-B operation
      return ((~beat_byte_A ^ ~beat_byte_B)-127)/127.0f;


    case 12:   // Special case: return "left" ByteBeat to optimize transition to regular crossfading, because we use the lower half for our logical operation
      return (beat_byte_A-127)/127.0f;


    default: // ByteBeatA as result again, just in case, as a "catch all" ;-)
      // printf("Encountered unexpected Id %d for bitwise operation on ByteBeats\n", logic_operation_id );
      return (beat_byte_A-127)/127.0f;
  }
}

void ctagSoundProcessorbbeats::Process(const ProcessData &data)
{
static uint16_t cv_counter = 0;   // A global counter for all instances, just to slow down checking of CV and controllers from GUI

  // --- List of lamdas, implementing the algorithms for Bytebeat 1 ---
  static uint8_t (*beats_P1[])(uint32_t t)  // Modify or add your own ByteBeats below!
  {
    [](uint32_t t) -> uint8_t { return (uint8_t)((t&128)); },                       // This is a basic square-wave, toggelling between 0 and 128
    [](uint32_t t) -> uint8_t { return (uint8_t)(1893*8); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(9893*t*(t/t*8)); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(9*t*(t/t*8)); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(9*t*(t/t*84)^990%t); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(9*t*(t/t*87)^990%t); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(t>>t|t<<245*2199); },
    [](uint32_t t) -> uint8_t { return (uint8_t)((502%t*t|19191/t)%552&t); },
    [](uint32_t t) -> uint8_t { return (uint8_t)((9/109)-t^t<<48); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(t*(t<<5|t>>7)); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(t%114|t%99); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(t%119^t%99); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(t<<119^t%99); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(t*120<<t%92); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(t*120<<t%90); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(t*1|t^119|t*99); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(t*(t%8|t>>3|t&400)^t); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(t<<((t<<t)|(t>>t))<<2); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(6-t&7|t<<999^t*212/t<<2); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(t^t%251); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(t^t%449); },
    [](uint32_t t) -> uint8_t { return (uint8_t)((t^t%449)+22); },
    [](uint32_t t) -> uint8_t { return (uint8_t)((t^t%249)-22); },
  };
  static const int beatA_max_idx = sizeof(beats_P1)/sizeof(beats_P1[0])-1;  // We calculate the number of list-entries, so that adding of algorithms does not need adjusting lenghts...

  // --- List of lamdas, implementing the algorithms for Bytebeat 2 ---
  static uint8_t (*beats_P2[])(uint32_t t)  // Modify or add your own ByteBeats below!
  {
    [](uint32_t t) -> uint8_t { return (uint8_t)(t&128); },                        // This is a basic square-wave, toggelling between 0 and 128
    [](uint32_t t) -> uint8_t { return (uint8_t)(1893*t&8); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(9893*t*(t/t*8)); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(9*t*(t/t*8)); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(9*t*(t/t*84)^990%t); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(9*t*(t/t*87)^990%t); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(t>>t|t<<245*2199); },
    [](uint32_t t) -> uint8_t { return (uint8_t)((502%t*t|19191/t)%552&t); },
    [](uint32_t t) -> uint8_t { return (uint8_t)((9/109)-t^t<<48); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(t*(t<<5|t>>7)); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(t%114|t%99); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(t%119^t%99); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(t<<119^t%99); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(t*120<<t%92); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(t*120<<t%90); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(t*1|t^119|t*99); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(t*(t%8|t>>3|t&400)^t); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(t<<((t<<t)|(t>>t))<<2); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(6-t&7|t<<999^t*212/t<<2); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(t^t%251); },
    [](uint32_t t) -> uint8_t { return (uint8_t)(t^t%449); },
    [](uint32_t t) -> uint8_t { return (uint8_t)((t^t%449)+22); },
    [](uint32_t t) -> uint8_t { return (uint8_t)((t^t%249)-22); },
  };
  static const int beatB_max_idx = sizeof(beats_P2)/sizeof(beats_P2[0])-1; // We calculate the number of list-entries, so that adding of algorithms does not need adjusting lenghts...

  // --- Read controllers from GUI or CV about every 5 millisecond and buffer results as private member variables ---
  if( ++cv_counter%220 == 0 )   // To optimize speed this is a static variable for all instances
  {
    // --- Read and buffer controllers for ByteBeat A ---
    stop_beatA = process_param_bool( data, trig_beatA_stop, beatA_stop );
    reset_beatA = process_param_bool( data, trig_beatA_reset_on_stop, beatA_reset_on_stop );
    reverse_beatA = process_param_bool( data, trig_beatA_backwards, beatA_backwards );
    if( stop_beatA && reset_beatA )
      reverse_beatA ? t1 = -1 : t1 = 1;      // reset incrementor for bytebeat algorithms, avoid 0 to not devide by zero
    beat_index_A = process_param( data,cv_beatA_select, beatA_select, beatA_max_idx, 22 );
    slow_down_A_factor = 129 - process_param( data,cv_beatA_pitch, beatA_pitch, 128, 128 );

    // --- Read and buffer controllers for ByteBeat B ---
    stop_beatB = process_param_bool( data, trig_beatB_stop, beatB_stop );
    reset_beatB = process_param_bool( data, trig_beatB_reset_on_stop, beatB_reset_on_stop );
    reverse_beatB = process_param_bool( data, trig_beatB_backwards, beatB_backwards );
    if( stop_beatB && reset_beatB )
      reverse_beatB ? t2 = -1 : t2 = 1;    // reset incrementor for bytebeat algorithms, avoid 0 to not devide by zero
    beat_index_B = process_param( data,cv_beatB_select, beatB_select, beatB_max_idx, 22 );
    slow_down_B_factor = 129 - process_param( data,cv_beatB_pitch, beatB_pitch, 128, 128 );

    // --- Read and buffer controllers for mixing ByteBeat A with ByteBeat B ---
    logic_mixes_allowed = process_param_bool(data, trig_allow_logic_mixes, allow_logic_mixes);
    if( logic_mixes_allowed )
    {
      if (cv_xFadeA_B != -1)
        data.cv[cv_xFadeA_B] >= 0.0  ? xfade_val_int = (int)(data.cv[cv_xFadeA_B] * 4095) : xfade_val_int = 0;
      else
        xfade_val_int = xFadeA_B;

      if( xfade_val_int < 2048 )                       // Use logical operation in first half of range of pot
        logic_operation_id = (int)(xfade_val_int * 13 / 2047);  // We multiply with 13 instead of normally 12, to avoid rounding errors with narrow range here
      else
        xfade_val = (xfade_val_int-2048)/2047.0f;        // We expand 50%-100% of pot to 0-100% for normal crossfade
    }
    else
      xfade_val = process_param_float(data, cv_xFadeA_B, xFadeA_B, 4095);
  }
  // --- This is our main loop, where the generation and mixing of ByteBeats takes place ---
  for (uint32_t i = 0; i < bufSz; i++)
  {
    // --- Process ByteBeat A ---
    if (slow_down_A >= slow_down_A_factor)
    {
      slow_down_A = 0;
      if( t1==0 )
        reverse_beatA ? t1-- : t1++;    // Avoid devision by zero for some algos
      if( !stop_beatA )
      {
        beat_byte_A = beats_P1[beat_index_A](t1);       // We may will also need the numeric value for logic operations on the ByteBeats
        beat_val_A = (float) (beat_byte_A - 127) / 127.0f; // beat_val_A: private member, so we buffer the result
        reverse_beatA ? t1-- : t1++;   // Decrement or increment iterator for ByteBeat1 algorithm
      }
    }
    slow_down_A++;  // We increment a counter for Beat1 every loop, so we can decide with next loop if we generate a new valur

    // --- Process ByteBeat B ---
    if (slow_down_B >= slow_down_B_factor)
    {
      slow_down_B = 0;
      if( t2==0 )
        reverse_beatA ? t2-- : t2++;    // Avoid devision by zero for some algos
      if( !stop_beatB )
      {
        beat_byte_B = beats_P2[beat_index_B](t2);       // We may will also need the numeric value for logic operations on the ByteBeats
        beat_val_B = (float)(beat_byte_B - 127) / 127.0f; // beat_val_B: private member, so we buffer the result
        reverse_beatB ? t2-- : t2++;   // Decrement or increment iterator for ByteBeat1 algorithm
      }
    }
    slow_down_B++;  // We increment a counter for Beat1 every loop, so we can decide with next loop if we generate a new valur

    // --- Mix ByteBeat A and ByteBeat B (in the first quarter of the Pot's range optionally logical operations may be performed on the beats  ---
    if( logic_mixes_allowed == true )     // Check for normal crossfade or additional logic operations for Pot for first quarter of range...
    {
      if( xfade_val_int <= 2047 )         // We are in the lower 50% of the Pot or GUI slider
        data.buf[i*2 + processCh] = logic_operation_on_beat();                              // Perform logical operations on ByteBeats in first 50% of PorRange for XFade!
      else                                                                                    // We already rescaled the xfade_val when reading controllers!
        data.buf[i*2 + processCh] = beat_val_A*(1.0f-xfade_val) + beat_val_B*xfade_val;        // Mix both ByteBeats, depending on XFade-factor
    }
    else
      data.buf[i*2 + processCh] = beat_val_A*(1.0f-xfade_val) + beat_val_B*xfade_val;         // Mix both ByteBeats, depending on XFade-factor
  }
}

ctagSoundProcessorbbeats::ctagSoundProcessorbbeats()
{
  // construct internal data model
  knowYourself();
  model = std::make_unique<ctagSPDataModel>(id, isStereo);
  LoadPreset(0);
}

ctagSoundProcessorbbeats::~ctagSoundProcessorbbeats()
{
}

void ctagSoundProcessorbbeats::knowYourself()
{
  // autogenerated code here
  // sectionCpp0
  pMapPar.emplace("beatA_reset_on_stop", [&](const int val){ beatA_reset_on_stop = val;});
  pMapTrig.emplace("beatA_reset_on_stop", [&](const int val){ trig_beatA_reset_on_stop = val;});
  pMapPar.emplace("beatA_stop", [&](const int val){ beatA_stop = val;});
  pMapTrig.emplace("beatA_stop", [&](const int val){ trig_beatA_stop = val;});
  pMapPar.emplace("beatA_backwards", [&](const int val){ beatA_backwards = val;});
  pMapTrig.emplace("beatA_backwards", [&](const int val){ trig_beatA_backwards = val;});
  pMapPar.emplace("beatA_select", [&](const int val){ beatA_select = val;});
  pMapCv.emplace("beatA_select", [&](const int val){ cv_beatA_select = val;});
  pMapPar.emplace("beatA_pitch", [&](const int val){ beatA_pitch = val;});
  pMapCv.emplace("beatA_pitch", [&](const int val){ cv_beatA_pitch = val;});
  pMapPar.emplace("beatB_reset_on_stop", [&](const int val){ beatB_reset_on_stop = val;});
  pMapTrig.emplace("beatB_reset_on_stop", [&](const int val){ trig_beatB_reset_on_stop = val;});
  pMapPar.emplace("beatB_stop", [&](const int val){ beatB_stop = val;});
  pMapTrig.emplace("beatB_stop", [&](const int val){ trig_beatB_stop = val;});
  pMapPar.emplace("beatB_backwards", [&](const int val){ beatB_backwards = val;});
  pMapTrig.emplace("beatB_backwards", [&](const int val){ trig_beatB_backwards = val;});
  pMapPar.emplace("beatB_select", [&](const int val){ beatB_select = val;});
  pMapCv.emplace("beatB_select", [&](const int val){ cv_beatB_select = val;});
  pMapPar.emplace("beatB_pitch", [&](const int val){ beatB_pitch = val;});
  pMapCv.emplace("beatB_pitch", [&](const int val){ cv_beatB_pitch = val;});
  pMapPar.emplace("allow_logic_mixes", [&](const int val){ allow_logic_mixes = val;});
  pMapTrig.emplace("allow_logic_mixes", [&](const int val){ trig_allow_logic_mixes = val;});
  pMapPar.emplace("xFadeA_B", [&](const int val){ xFadeA_B = val;});
  pMapCv.emplace("xFadeA_B", [&](const int val){ cv_xFadeA_B = val;});

  isStereo = false;
  id = "bbeats";
  // sectionCpp0
}