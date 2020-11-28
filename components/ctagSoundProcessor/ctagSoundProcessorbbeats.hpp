#include <atomic>
#include "ctagSoundProcessor.hpp"

#define BEAT_A_MAX_IDX 54     // Max Index for list of ByteBeat 1
#define BEAT_B_MAX_IDX 55     // Max Index for list of ByteBeat 2


namespace CTAG {
    namespace SP {
        class ctagSoundProcessorbbeats : public ctagSoundProcessor {
        public:
            virtual void Process(const ProcessData &) override;
            ctagSoundProcessorbbeats();
            virtual ~ctagSoundProcessorbbeats();

        private:
            virtual void knowYourself() override;

            inline static int process_param( const ProcessData &data, int cv_myparm, int my_parm, int parm_range, int max_idx ); // rescale incoming data
            inline static float process_param_float( const ProcessData &data, int cv_myparm, int my_parm, int max_idx ); // rescale incoming data to 0.0-1.0
            inline bool process_param_bool( const ProcessData &data, int trig_myparm, int my_parm, int prev_trig_state_id ); // rescale incoming data to bool
            inline float logic_operation_on_beat( );   // Logical operation on the bytebeats

            enum trig_states {e_stop_beatA, e_reset_beatA, e_reverse_beatA, e_stop_beatB, e_reset_beatB, e_reverse_beatB, e_logic_mixes_allowed, e_max };
            uint8_t prev_trig_state[e_max] = {1,1,1,1,1,1,1};
            uint16_t cv_counter = 0;    // A counter to slow down checking of CV and controllers from GUI

            // private attributes could go here
            uint8_t beat_byte_A = 0;  // Currently calculated or temporarily stored ByteBeatA
            uint8_t beat_byte_B = 0;  // Currently calculated or temporarily stored ByteBeatB
            float beat_val_A = 0.0f;      // Currently calculated or temporarily stored audio value for ByteBeatA
            float beat_val_B = 0.0f;      // Currently calculated or temporarily stored audio value for ByteBeatB
            uint32_t t1 = 0;             // Iterator for ByteBeat1
            uint32_t t2 = 0;             // Iterator for ByteBeat2

            bool stop_beatA = false;     // BeatA will not play if stopped
            bool reset_beatA = false;    // BeatB will play from start again on restart if true
            bool reverse_beatA = false;  // True if ByteBeat1 is meant to play backwards
            uint16_t slow_down_A = 0;    // Speed counter for ByteBeat1
            int slow_down_A_factor = 0;  // Speed factor for ByteBeat1

            bool stop_beatB = false;     // BeatB will not play if stopped
            bool reset_beatB = false;    // BeatB will play from start again on restart if true
            bool reverse_beatB = false;  // True if ByteBeat2 is meant to play backwards
            uint16_t slow_down_B = 0;    // Speed counter for ByteBeat2
            int slow_down_B_factor = 0;  // Speed factor for ByteBeat2

            int beat_index_A = 0;         // Used to decide which ByteBeat1 from the lists below has been selected by controller / CV
            int beat_index_B = 0;         // Used to decide which ByteBeat2 from the lists below has been selected by controller / CV

            bool logic_mixes_allowed = false;
            int logic_operation_id = 0;   // If we mix both ByteBeats using various logical operators, this is the index to select the operation
            float xfade_val = 0.0f;     // This is our value to Xfade between ByteBeatA and ByteBeatB
            int xfade_val_int = 0;      // We also need to remember the integer value of the Xfader to select logical operation on the first 25% of the fader if activated

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshift-count-overflow"
            // --- List of lamdas, implementing the algorithms for Bytebeat 1 ---
            static constexpr uint8_t (*beats_P1[BEAT_A_MAX_IDX+1])(uint32_t t)  // Modify or add your own ByteBeats below!
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
              [](uint32_t t) -> uint8_t { return (uint8_t)(t*((t>>12|t>>8)&63&t>>4)); },          // Matt's rhythmical beats start here
              [](uint32_t t) -> uint8_t { return (uint8_t)(t*((t>>12|t>>8)&61&t>>11)); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t*((t>>12|t>>8)&59&t>>9)); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(((t&15)*(-t&15)^!(t&16)-1)+32); },
              [](uint32_t t) -> uint8_t { return (uint8_t)((t%100>>t/200%100)*255); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(((43|4|25|66)/t>>89)+(t&2)); },
              [](uint32_t t) -> uint8_t { return (uint8_t)((t/11)*(t/16|6+7)/9); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(((t*3|t>>361)+139)<<49); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(((t%16)<<6)/8|t%255|t-100); },
              [](uint32_t t) -> uint8_t { return (uint8_t)((-t/100|(t*3))^(t*3&(t>>3))&t); },
              [](uint32_t t) -> uint8_t { return (uint8_t)((-t/100|(t&1))^(t*3&(t>>39))&t>>99); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t<<6/(8080+t)&900+t); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t<<2&(t*(7+(1^t>>6|9%5|4)))); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t%24^2&(t*(7+(1^t>>9|9%3|4)))); },
              [](uint32_t t) -> uint8_t { return (uint8_t)((t|(t>>9|t>>7))*t&(t>>11|t>>9)); },
              [](uint32_t t) -> uint8_t { return (uint8_t)((t|(t>>11|t>>55))^t%((t>>1|t>>22)|t>>9)); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t*3&(t>>7)+t*9&(t*4<<1)); }, // 20201128 MB: double entry to prevent Segfault as below!
              // ??? [](uint32_t t) -> uint8_t { return (uint8_t)(t|(t<<1|t<<2)%t%(t&302|t%2)); }, // ### ??? SEGFAULT encountered!
              [](uint32_t t) -> uint8_t { return (uint8_t)(t*3&(t>>7)+t*9&(t*4<<1)); }, // 20201128 MB: double entry to prevent Segfault as below!
              // ??? [](uint32_t t) -> uint8_t { return (uint8_t)((t|(t<<320|t<<9))%t%(t&309|t%2)); },   // ### ??? SEGFAULT encountered!
              [](uint32_t t) -> uint8_t { return (uint8_t)(t*3&(t>>7)+t*9&(t*4<<1)); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t*(t>>3)|80%t*t); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t>>t>>t>>t); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t>>t); },
              [](uint32_t t) -> uint8_t { return (uint8_t)((t&t)); },
              [](uint32_t t) -> uint8_t { return (uint8_t)((t-t^t)^t|t^202&t|t<<183); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t-246|t+9*t); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(((t^110/256)>>(t^956*8000/256&8)|t)); },
              [](uint32_t t) -> uint8_t { return (uint8_t)((t-t%t)|t%158/112>>91|(t%(41*241<<t))^t>>8&t); },
              [](uint32_t t) -> uint8_t { return (uint8_t)((t>>t+2)<<(8*t)*4); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t*3|t/3|t*99|t/2); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t*3|t/3|t*99); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(44>>(t>>t+2)<<(8*t)*4); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t&t&129|233-244%t>>t); },
            };
            // --- List of lamdas, implementing the algorithms for Bytebeat 2 ---
            static constexpr uint8_t (*beats_P2[BEAT_B_MAX_IDX+1])(uint32_t t)  // Modify or add your own ByteBeats below!
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
              [](uint32_t t) -> uint8_t { return (uint8_t)(t&t/1219); },                      // Matt's rhythmical beats start here
              [](uint32_t t) -> uint8_t { return (uint8_t)(4448-(t>>t^1)<<(8*t)*1); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t*t+2>>t&t|(t^9)+1); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t%255^t%64); },
              [](uint32_t t) -> uint8_t { return (uint8_t)((t%(t|2+t-39)*1)>>(t&28)); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t*3|t^3|t*99); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t*333|t>>3|t*1); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t*333|t>>3|t*11); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t*3333|t>>3|t*11); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t^3333|t>>18|t/113); },
              [](uint32_t t) -> uint8_t { return (uint8_t)((6622%t*484)); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(7-t%7|t<<999|t*212%t<<3); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t<<5|t>>2&t%99); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t<<(t>>t*169)|t>>t|t/20); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t<<5|t>>2-t%99); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t&t/269%t|t/223); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t&t/261&t|t/225); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t&t/1269%t&t/223); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t*(t<<1|t>>8)); },
              [](uint32_t t) -> uint8_t { return (uint8_t)((t%100&t/200%100)*200); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(7-t%7|t<<999|t*212%t<<2); },
              [](uint32_t t) -> uint8_t { return (uint8_t)((t/111*t>>18*t/999)|t<<2); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t*t/38%216); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t^(98&t)); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t^(93/t)); },
              [](uint32_t t) -> uint8_t { return (uint8_t)((t+39)|t-t|t-(25*t)); },
              [](uint32_t t) -> uint8_t { return (uint8_t)((t+39)|t&t|t-(22*t)); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t+208/109-t|t^t|97%t>>t); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t^57+t%149|t%(251&103)); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t/333|t>>18*t/999); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t/666*t>>18*t/999); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t<<5|t>>2+t%99); },
              [](uint32_t t) -> uint8_t { return (uint8_t)(t-85-(98>>t)|(22*80-t)*t|t/215); },
            };
#pragma GCC diagnostic pop
            // autogenerated code here
            // sectionHpp
            atomic<int32_t> beatA_reset_on_stop, trig_beatA_reset_on_stop;
            atomic<int32_t> beatA_stop, trig_beatA_stop;
            atomic<int32_t> beatA_backwards, trig_beatA_backwards;
            atomic<int32_t> beatA_select, cv_beatA_select;
            atomic<int32_t> beatA_pitch, cv_beatA_pitch;
            atomic<int32_t> beatB_reset_on_stop, trig_beatB_reset_on_stop;
            atomic<int32_t> beatB_stop, trig_beatB_stop;
            atomic<int32_t> beatB_backwards, trig_beatB_backwards;
            atomic<int32_t> beatB_select, cv_beatB_select;
            atomic<int32_t> beatB_pitch, cv_beatB_pitch;
            atomic<int32_t> allow_logic_mixes, trig_allow_logic_mixes;
            atomic<int32_t> xFadeA_B, cv_xFadeA_B;
            atomic<int32_t> destination, cv_destination;
            atomic<int32_t> loopEG, trig_loopEG;
            atomic<int32_t> attack, cv_attack;
            atomic<int32_t> decay, cv_decay;
            // sectionHpp
        };
    }
}