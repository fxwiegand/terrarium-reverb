#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

// Declare a local daisy_petal for hardware access
DaisyPetal hw;

Parameter vtime, vfreq, vsend, lfo_speed, amplitude;
bool      bypass;
ReverbSc  verb;
Oscillator lfo;

Led led1, led2;

// This runs at a fixed rate, to prepare audio samples
void callback(float *in, float *out, size_t size)
{
    float dryl, dryr, wetl, wetr, sendl, sendr;
    hw.ProcessAllControls();
    led1.Update();
    led2.Update();

    lfo.SetFreq(lfo_speed.Process() * 5.0f);
    lfo.SetAmp(amplitude.Process());

    if (hw.switches[Terrarium::SWITCH_2].Pressed()) {
        lfo.SetWaveform(Oscillator::WAVE_SIN);
    } else {
        lfo.SetWaveform(Oscillator::WAVE_TRI);
    }

    verb.SetFeedback(vtime.Process());
    verb.SetLpFreq(vfreq.Process());

    vsend.Process();

    if(hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        bypass = !bypass;
        led1.Set(bypass ? 0.0f : 1.0f);
    }

    for(size_t i = 0; i < size; i += 2)
    {
        dryl  = in[i];
        dryr  = in[i + 1];
        sendl = dryl * vsend.Value();
        sendr = dryr * vsend.Value();
        verb.Process(sendl, sendr, &wetl, &wetr);

        if(bypass)
        {
            out[i]     = in[i];     // left
            out[i + 1] = in[i + 1]; // right
        }
        else
        {
            if (hw.switches[Terrarium::SWITCH_1].Pressed()) {
                out[i] = dryl + (wetl * lfo.Process());
                out[i + 1] = dryr + (wetr * lfo.Process());
            } else {
                out[i] = dryl + wetl;
                out[i + 1] = dryr + wetr;
            }
        }
    }
}

int main(void)
{
    float samplerate;

    hw.Init();
    samplerate = hw.AudioSampleRate();
    hw.SetAudioBlockSize(12);

    vtime.Init(hw.knob[Terrarium::KNOB_1], 0.6f, 0.999f, Parameter::LOGARITHMIC);
    vfreq.Init(hw.knob[Terrarium::KNOB_2], 500.0f, 20000.0f, Parameter::LOGARITHMIC);
    vsend.Init(hw.knob[Terrarium::KNOB_3], 0.0f, 1.0f, Parameter::LINEAR);
    lfo_speed.Init(hw.knob[Terrarium::KNOB_4], 0.005f, 0.15f, Parameter::LOGARITHMIC);
    amplitude.Init(hw.knob[Terrarium::KNOB_5], 0.65f, 0.999f, Parameter::LINEAR);
    verb.Init(samplerate);

    lfo.Init(samplerate);

    led1.Init(hw.seed.GetPin(Terrarium::LED_1),false);
    led2.Init(hw.seed.GetPin(Terrarium::LED_2),false, 10000.0f);
    led1.Update();
    bypass = true;

    hw.StartAdc();
    hw.StartAudio(callback);
    while(1)
    {
        // Show LFO rate with second LED if modulation is on
        led2.Set(lfo.Process());
        led2.Update();
    }
}