#include "daisy_seed.h"
#include "daisysp.h"
#include "adsr.h"

// Use the daisy namespace to prevent having to type
// daisy:: before all libdaisy functions
using namespace daisy;

// Declare a DaisySeed object called hardware
DaisySeed hardware;
MidiUsbHandler midi;
Adsr adsr; //TODO: explore daisysp adsr
uint16_t pitch;

static constexpr size_t dac_buf_len = 48;
uint16_t DMA_BUFFER_MEM_SECTION dac_buf[2][dac_buf_len];

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{

}

void my_dac_callback(uint16_t **out, size_t size) {
	uint16_t adsr_value = adsr.step() * 0xFFF;
    for(size_t i = 0; i < size; i++) {
        out[0][i] = pitch;
        out[1][i] = adsr_value;
    }
}

int main(void)
{
    hardware.Configure();
    hardware.Init();

	MidiUsbHandler::Config midi_cfg;
    midi_cfg.transport_config.periph = MidiUsbTransport::Config::INTERNAL;
    midi.Init(midi_cfg);

	DacHandle::Config dac_cfg;
	dac_cfg.target_samplerate = 48000; //16kHz
	dac_cfg.bitdepth   = DacHandle::BitDepth::BITS_12;
	dac_cfg.buff_state = DacHandle::BufferState::ENABLED;
	dac_cfg.mode       = DacHandle::Mode::DMA;
	dac_cfg.chn        = DacHandle::Channel::BOTH;
	hardware.dac.Init(dac_cfg);
	hardware.dac.Start(dac_buf[0], dac_buf[1], dac_buf_len, my_dac_callback);

    AdcChannelConfig adc_cfg[4];
    adc_cfg[0].InitSingle(hardware.GetPin(18));
    adc_cfg[1].InitSingle(hardware.GetPin(19));
    adc_cfg[2].InitSingle(hardware.GetPin(21));
    adc_cfg[3].InitSingle(hardware.GetPin(24));
    hardware.adc.Init(&adc_cfg[0], 4);

	Led led[8];
	for(int i = 0; i < 8; i++) {
		led[i].Init(hardware.GetPin(i+7), false);
		led[i].Set(0.0f);
	}

    hardware.adc.Start();

    while(1)
    {
        midi.Listen();
        while(midi.HasEvents())
        {
            auto msg = midi.PopEvent();
            switch(msg.type)
            {
                case NoteOn:
                {
					auto note_msg = msg.AsNoteOn();
					if(note_msg.velocity != 0) {
						adsr.params.attack_time = hardware.adc.GetFloat(0);
						adsr.params.decay_time = hardware.adc.GetFloat(1);
						adsr.params.sustain_amt = hardware.adc.GetFloat(2);
						adsr.params.release_time = hardware.adc.GetFloat(3);
						adsr.trigger();
						float freq = daisysp::mtof(note_msg.note);
						float L = 220;
						float H = 880;
						float m = 1.0f/(H-L);
						float b = 1.0f/((H/L)-1.0f);
						pitch = ((m*freq + b) * 2.0f) * 0xFFF;
					}
                }
                break;
                default: break;
            }
        }
    }
}
