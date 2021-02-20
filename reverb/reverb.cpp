#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

// Declare a local daisy_petal for hardware access
DaisyPetal hw;

Parameter vtime, vfreq, vsend;
bool      bypass;
ReverbSc  verb;

Led led1, led2;

// This runs at a fixed rate, to prepare audio samples
void callback(float *in, float *out, size_t size)
{
    float dryl, dryr, wetl, wetr, sendl, sendr;
    hw.ProcessAllControls();
    led1.Update();
    led2.Update();

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
            out[i]     = dryl + wetl;
            out[i + 1] = dryr + wetr;
        }
    }
}

int main(void)
{
    float samplerate;

    hw.Init();
    samplerate = hw.AudioSampleRate();

    vtime.Init(hw.knob[Terrarium::KNOB_1], 0.6f, 0.999f, Parameter::LOGARITHMIC);
    vfreq.Init(hw.knob[Terrarium::KNOB_2], 500.0f, 20000.0f, Parameter::LOGARITHMIC);
    vsend.Init(hw.knob[Terrarium::KNOB_3], 0.0f, 1.0f, Parameter::LINEAR);
    verb.Init(samplerate);

    led1.Init(hw.seed.GetPin(Terrarium::LED_1),false);
    led1.Update();
    bypass = true;

    hw.StartAdc();
    hw.StartAudio(callback);
    while(1)
    {
        // Do Stuff InfInitely Here
        System::Delay(10);
    }
}