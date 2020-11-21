/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/


#pragma once

#include <memory>
#include <iostream>
#include <string>
#include "ctagSoundProcessor.hpp"
#include "ctagSoundProcessors.hpp"

namespace CTAG {
    namespace SP {
        class ctagSoundProcessorFactory {
        public:
            static std::unique_ptr <ctagSoundProcessor> Create(const std::string type) {
// generated code
                if(type.compare("BCSR") == 0) return std::make_unique<ctagSoundProcessorBCSR>();
if(type.compare("CDelay") == 0) return std::make_unique<ctagSoundProcessorCDelay>();
if(type.compare("DLoop") == 0) return std::make_unique<ctagSoundProcessorDLoop>();
if(type.compare("Dust") == 0) return std::make_unique<ctagSoundProcessorDust>();
if(type.compare("FBDlyLine") == 0) return std::make_unique<ctagSoundProcessorFBDlyLine>();
if(type.compare("FVerb") == 0) return std::make_unique<ctagSoundProcessorFVerb>();
if(type.compare("GDVerb") == 0) return std::make_unique<ctagSoundProcessorGDVerb>();
if(type.compare("GDVerb2") == 0) return std::make_unique<ctagSoundProcessorGDVerb2>();
if(type.compare("GVerb") == 0) return std::make_unique<ctagSoundProcessorGVerb>();
if(type.compare("Hihat1") == 0) return std::make_unique<ctagSoundProcessorHihat1>();
if(type.compare("MIChorus") == 0) return std::make_unique<ctagSoundProcessorMIChorus>();
if(type.compare("MIDifu") == 0) return std::make_unique<ctagSoundProcessorMIDifu>();
if(type.compare("MIEnsemble") == 0) return std::make_unique<ctagSoundProcessorMIEnsemble>();
if(type.compare("MIPShft") == 0) return std::make_unique<ctagSoundProcessorMIPShft>();
if(type.compare("MISVF") == 0) return std::make_unique<ctagSoundProcessorMISVF>();
if(type.compare("MIVerb") == 0) return std::make_unique<ctagSoundProcessorMIVerb>();
if(type.compare("MIVerb2") == 0) return std::make_unique<ctagSoundProcessorMIVerb2>();
if(type.compare("MacOsc") == 0) return std::make_unique<ctagSoundProcessorMacOsc>();
if(type.compare("MoogFilt") == 0) return std::make_unique<ctagSoundProcessorMoogFilt>();
if(type.compare("PNoise") == 0) return std::make_unique<ctagSoundProcessorPNoise>();
if(type.compare("PolyPad") == 0) return std::make_unique<ctagSoundProcessorPolyPad>();
if(type.compare("SimpleVCA") == 0) return std::make_unique<ctagSoundProcessorSimpleVCA>();
if(type.compare("SineSrc") == 0) return std::make_unique<ctagSoundProcessorSineSrc>();
if(type.compare("StrampDly") == 0) return std::make_unique<ctagSoundProcessorStrampDly>();
if(type.compare("SubSynth") == 0) return std::make_unique<ctagSoundProcessorSubSynth>();
if(type.compare("TBD03") == 0) return std::make_unique<ctagSoundProcessorTBD03>();
if(type.compare("TBDaits") == 0) return std::make_unique<ctagSoundProcessorTBDaits>();
if(type.compare("TBDeep") == 0) return std::make_unique<ctagSoundProcessorTBDeep>();
if(type.compare("TBDings") == 0) return std::make_unique<ctagSoundProcessorTBDings>();
if(type.compare("bbeats") == 0) return std::make_unique<ctagSoundProcessorbbeats>();

// end generated code
                return nullptr;
            }
        };
    }
}
