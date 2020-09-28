#include <atomic>
#include "ctagSoundProcessor.hpp"
#include "polypad/ChordSynth.hpp"
#include <vector>
#include "braids/settings.h"
#include "braids/quantizer.h"

namespace CTAG {
    namespace SP {
        class ctagSoundProcessorPolyPad : public ctagSoundProcessor {
        public:
            virtual void Process(const ProcessData &) override;

            virtual ~ctagSoundProcessorPolyPad();

            ctagSoundProcessorPolyPad();
            void PrintParams(ChordSynth::ChordParams &params);

            virtual const char *GetCStrID() const override;

        private:
            void setParamValueInternal(const string &id, const string &key, const int val) override;
            void loadPresetInternal() override;

// autogenerated code here
// sectionHpp
const string id = "PolyPad";
void setIsStereo(){isStereo = false;}
atomic<int32_t> gain, cv_gain;
atomic<int32_t> pitch, cv_pitch;
atomic<int32_t> q_scale, cv_q_scale;
atomic<int32_t> chord, cv_chord;
atomic<int32_t> inversion, cv_inversion;
atomic<int32_t> detune, cv_detune;
atomic<int32_t> nnotes, cv_nnotes;
atomic<int32_t> ncvoices, cv_ncvoices;
atomic<int32_t> voicehold, trig_voicehold;
atomic<int32_t> lfo1_freq, cv_lfo1_freq;
atomic<int32_t> lfo1_amt, cv_lfo1_amt;
atomic<int32_t> filter_type, cv_filter_type;
atomic<int32_t> cutoff, cv_cutoff;
atomic<int32_t> resonance, cv_resonance;
atomic<int32_t> lfo2_freq, cv_lfo2_freq;
atomic<int32_t> lfo2_amt, cv_lfo2_amt;
atomic<int32_t> lfo2_rphase, trig_lfo2_rphase;
atomic<int32_t> eg_filt_amt, cv_eg_filt_amt;
atomic<int32_t> enableEG, trig_enableEG;
atomic<int32_t> latchEG, trig_latchEG;
atomic<int32_t> eg_slow_fast, trig_eg_slow_fast;
atomic<int32_t> attack, cv_attack;
atomic<int32_t> decay, cv_decay;
atomic<int32_t> sustain, cv_sustain;
atomic<int32_t> release, cv_release;
// sectionHpp




            // private attributes could go here
            vector<ChordSynth> v_voices;
            bool latchVoice = false;
            bool latched = false;
            bool toggle = false;
            braids::Quantizer quantizer;
        };
    }
}