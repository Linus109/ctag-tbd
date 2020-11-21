#include "ctagSoundProcessorbbeats.hpp"

using namespace CTAG::SP;

// data.buf[i * 2 + processCh] = (float)((byte)(t & 128));    // Square wave, working ok!

// --- Helper function: rescale CV or Pot to integer of given range 0...max ---
int ctagSoundProcessorbbeats::process_param( const ProcessData &data, int cv_myparm, int my_parm, int parm_range )
{
    return (cv_myparm != -1) ? (int)(data.cv[cv_myparm] * parm_range) : my_parm * parm_range / 4095;
}
// --- Helper function: rescale CV or Pot to float 0...1.0 (CV is already in correct format, we still keep it inside this method for convenience ---
float ctagSoundProcessorbbeats::process_param_float( const ProcessData &data, int cv_myparm, int my_parm )
{
    return (cv_myparm != -1) ? data.cv[cv_myparm] : my_parm/4095.0;
}

void ctagSoundProcessorbbeats::Process(const ProcessData &data)
{
    float xfade_val = 0;                   // Use to setup crossfade between our two bytebeats...
    static byte (*beats_P1[])(uint32_t t)  // List of Bytebeats for P1, Modify or add your own ByteBeats below!
    {
        [](uint32_t t) -> byte { return (byte)((t&128)); },                       // This is a basic square-wave, toggelling between 0 and 128
        [](uint32_t t) -> byte { return (byte)(1893*8); },
        [](uint32_t t) -> byte { return (byte)(9893*t*(t/t*8)); },
        [](uint32_t t) -> byte { return (byte)(9*t*(t/t*8)); },
        [](uint32_t t) -> byte { return (byte)(9*t*(t/t*84)^990%t); },
        [](uint32_t t) -> byte { return (byte)(9*t*(t/t*87)^990%t); },
        [](uint32_t t) -> byte { return (byte)(t>>t|t<<245*2199); },
        [](uint32_t t) -> byte { return (byte)((502%t*t|19191/t)%552&t); },
        [](uint32_t t) -> byte { return (byte)((9/109)-t^t<<48); },
        [](uint32_t t) -> byte { return (byte)(t*(t<<5|t>>7)); },
        [](uint32_t t) -> byte { return (byte)(t%114|t%99); },
        [](uint32_t t) -> byte { return (byte)(t%119^t%99); },
        [](uint32_t t) -> byte { return (byte)(t<<119^t%99); },
        [](uint32_t t) -> byte { return (byte)(t*120<<t%92); },
        [](uint32_t t) -> byte { return (byte)(t*120<<t%90); },
        [](uint32_t t) -> byte { return (byte)(t*1|t^119|t*99); },
        [](uint32_t t) -> byte { return (byte)(t*(t%8|t>>3|t&400)^t); },
        [](uint32_t t) -> byte { return (byte)(t<<((t<<t)|(t>>t))<<2); },
        [](uint32_t t) -> byte { return (byte)(6-t&7|t<<999^t*212/t<<2); },
        [](uint32_t t) -> byte { return (byte)(t^t%251); },
        [](uint32_t t) -> byte { return (byte)(t^t%449); },
        [](uint32_t t) -> byte { return (byte)((t^t%449)+22); },
        [](uint32_t t) -> byte { return (byte)((t^t%249)-22); },
    };
    static const int beatA_max_idx = sizeof(beats_P1)/sizeof(beats_P1[0])-1;

    static byte (*beats_P2[])(uint32_t t)  // List of Bytebeats for P2, Modify or add your own ByteBeats below!
    {
        [](uint32_t t) -> byte { return (byte)(t&128); },                       // This is a basic square-wave, toggelling between 0 and 128
        [](uint32_t t) -> byte { return (byte)(1893*t&8); },
        [](uint32_t t) -> byte { return (byte)(9893*t*(t/t*8)); },
        [](uint32_t t) -> byte { return (byte)(9*t*(t/t*8)); },
        [](uint32_t t) -> byte { return (byte)(9*t*(t/t*84)^990%t); },
        [](uint32_t t) -> byte { return (byte)(9*t*(t/t*87)^990%t); },
        [](uint32_t t) -> byte { return (byte)(t>>t|t<<245*2199); },
        [](uint32_t t) -> byte { return (byte)((502%t*t|19191/t)%552&t); },
        [](uint32_t t) -> byte { return (byte)((9/109)-t^t<<48); },
        [](uint32_t t) -> byte { return (byte)(t*(t<<5|t>>7)); },
        [](uint32_t t) -> byte { return (byte)(t%114|t%99); },
        [](uint32_t t) -> byte { return (byte)(t%119^t%99); },
        [](uint32_t t) -> byte { return (byte)(t<<119^t%99); },
        [](uint32_t t) -> byte { return (byte)(t*120<<t%92); },
        [](uint32_t t) -> byte { return (byte)(t*120<<t%90); },
        [](uint32_t t) -> byte { return (byte)(t*1|t^119|t*99); },
        [](uint32_t t) -> byte { return (byte)(t*(t%8|t>>3|t&400)^t); },
        [](uint32_t t) -> byte { return (byte)(t<<((t<<t)|(t>>t))<<2); },
        [](uint32_t t) -> byte { return (byte)(6-t&7|t<<999^t*212/t<<2); },
        [](uint32_t t) -> byte { return (byte)(t^t%251); },
        [](uint32_t t) -> byte { return (byte)(t^t%449); },
        [](uint32_t t) -> byte { return (byte)((t^t%449)+22); },
        [](uint32_t t) -> byte { return (byte)((t^t%249)-22); },
    };
    static const int beatB_max_idx = sizeof(beats_P2)/sizeof(beats_P2[0])-1;

    for (uint32_t i = 0; i < bufSz; i++)
    {
        slow_down_A_factor = 129 - process_param( data,cv_beatA_pitch, beatA_pitch, 128 );
        if (slow_down_A % slow_down_A_factor == 0)      // slow_down_A is unsigned, so it wraps around on overflow!
        {
            t1++;   // Increment iterator for ByteBeat1 algorithm
            beat_index_A = process_param( data,cv_beatA_select, beatA_select, beatA_max_idx );
            beat_val_A = (float)((int) beats_P1[beat_index_A](t1) - 127) / 127.0; // beat_val: private member, so we buffer the result
        }
        slow_down_A++;  // We increment a counter for Beat1 every loop, so we can decide with next loop if we generate a new valur

        slow_down_B_factor = 129 - process_param( data,cv_beatB_pitch, beatB_pitch, 128 );
        if (slow_down_B % slow_down_B_factor == 0)      // slow_down_B is unsigned, so it wraps around on overflow!
        {
            t2++;   // Increment iterator for ByteBeat1 algorithm
            beat_index_B = process_param( data,cv_beatB_select, beatB_select, beatB_max_idx );
            beat_val_B = (float)((int) beats_P2[beat_index_B](t1) - 127) / 127.0; // beat_val: private member, so we buffer the result
        }
        slow_down_B++;  // We increment a counter for Beat1 every loop, so we can decide with next loop if we generate a new valur

        xfade_val = process_param_float( data,cv_xFadeA_B, xFadeA_B);
        // if( t2 % 1000000 )
        //     printf("xfade_val: %f \n", xfade_val);
        data.buf[i*2 + processCh] = beat_val_A*(1.0-xfade_val) + beat_val_B*xfade_val;         // Mix both ByteBeats, depending on XFade-factor
    }
}

ctagSoundProcessorbbeats::ctagSoundProcessorbbeats() {
    // construct internal data model
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);
}

ctagSoundProcessorbbeats::~ctagSoundProcessorbbeats() {
}

void ctagSoundProcessorbbeats::knowYourself(){
    // autogenerated code here
    // sectionCpp0
    pMapPar.emplace("beatA_stop", [&](const int val){ beatA_stop = val;});
    pMapTrig.emplace("beatA_stop", [&](const int val){ trig_beatA_stop = val;});
    pMapPar.emplace("beatA_reset_on_stop", [&](const int val){ beatA_reset_on_stop = val;});
    pMapTrig.emplace("beatA_reset_on_stop", [&](const int val){ trig_beatA_reset_on_stop = val;});
    pMapPar.emplace("beatB_stop", [&](const int val){ beatB_stop = val;});
    pMapTrig.emplace("beatB_stop", [&](const int val){ trig_beatB_stop = val;});
    pMapPar.emplace("beatB_reset_on_stop", [&](const int val){ beatB_reset_on_stop = val;});
    pMapTrig.emplace("beatB_reset_on_stop", [&](const int val){ trig_beatB_reset_on_stop = val;});
    pMapPar.emplace("beatA_backwards", [&](const int val){ beatA_backwards = val;});
    pMapTrig.emplace("beatA_backwards", [&](const int val){ trig_beatA_backwards = val;});
    pMapPar.emplace("beatB_backwards", [&](const int val){ beatB_backwards = val;});
    pMapTrig.emplace("beatB_backwards", [&](const int val){ trig_beatB_backwards = val;});
    pMapPar.emplace("beatA_select", [&](const int val){ beatA_select = val;});
    pMapCv.emplace("beatA_select", [&](const int val){ cv_beatA_select = val;});
    pMapPar.emplace("beatB_select", [&](const int val){ beatB_select = val;});
    pMapCv.emplace("beatB_select", [&](const int val){ cv_beatB_select = val;});
    pMapPar.emplace("beatA_pitch", [&](const int val){ beatA_pitch = val;});
    pMapCv.emplace("beatA_pitch", [&](const int val){ cv_beatA_pitch = val;});
    pMapPar.emplace("beatB_pitch", [&](const int val){ beatB_pitch = val;});
    pMapCv.emplace("beatB_pitch", [&](const int val){ cv_beatB_pitch = val;});
    pMapPar.emplace("xFadeA_B", [&](const int val){ xFadeA_B = val;});
    pMapCv.emplace("xFadeA_B", [&](const int val){ cv_xFadeA_B = val;});

    isStereo = false;
    id = "bbeats";
	// sectionCpp0
}