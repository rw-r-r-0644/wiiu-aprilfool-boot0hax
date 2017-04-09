#include "crash_tone.h"

#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/ax_functions.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

/* Based on QuarkTheAwesome's LiveSynthesisU :) */

voiceData voice1;
ax_buffer_t voiceBuffer;

void axFrameCallback()
{
	if (voice1.stopRequested)
	{
		//Stop the voice
		AXSetVoiceState(voice1.voice, 0);
		//Clear the flag
		voice1.stopRequested = 0;
		voice1.stopped = 1;
		DCFlushRange(&voice1, sizeof(voice1));
	}
	else if (voice1.modified)
	{
		//does this really need to happen in the callback?
		DCInvalidateRange(&voice1, sizeof(voice1));
		memset(&voiceBuffer, 0, sizeof(voiceBuffer));
		voiceBuffer.samples = voice1.samples;
		voiceBuffer.format = 25; //almost definitely wrong
		voiceBuffer.loop = 1;
		voiceBuffer.cur_pos = 0;
		voiceBuffer.end_pos = voice1.numSamples - 1;
		voiceBuffer.loop_offset = 0;
	
		u32 ratioBits[4];
		ratioBits[0] = (u32)(0x00010000 * ((float)48000 / (float)AXGetInputSamplesPerSec()));
		ratioBits[1] = 0;
		ratioBits[2] = 0;
		ratioBits[3] = 0;
	
		AXSetVoiceOffsets(voice1.voice, &voiceBuffer);
		AXSetVoiceSrc(voice1.voice, ratioBits);
		AXSetVoiceSrcType(voice1.voice, 1);
		AXSetVoiceState(voice1.voice, 1);
		voice1.modified = 0;
		voice1.stopped = 0;
		DCFlushRange(&voice1, sizeof(voice1));
	} 
}

u32 generateSawtoothWave(sample* samples, u32 maxLength, float freq)
{
	int wavelengthSamples = (int)(48000 / (freq)); //number of samples in one wavelength
	char step = (char)((AMPLITUDE * 2) / wavelengthSamples); //"step" in amplitude per sample
	char tempSample = (char)-AMPLITUDE;
	int i = 0;
	for (; i < wavelengthSamples; i++)
	{
		tempSample += step;
		samples[i] = (sample)tempSample;
	}
	return i;
}

void crash_with_sound()
{
	/*!
		Initialize AX library and set up voice1
	*/	
	//init 48KHz renderer
	u32 params[3] = {1, 0, 0};
	AXInitWithParams(params);
	AXRegisterFrameCallback((void*)axFrameCallback); //this callback doesn't really do much
	
	memset(&voice1, 0, sizeof(voice1));
	voice1.samples = malloc(SAMPLE_BUFFER_MAX_SIZE); //allocate room for samples
	
	voice1.stopped = 1;
	
	voice1.voice = AXAcquireVoice(25, 0, 0); //get a voice, priority 25. Just a random number I picked
	if (!voice1.voice)
		return;
	
	AXVoiceBegin(voice1.voice);
	AXSetVoiceType(voice1.voice, 0);
	
	//Set volume?
	u32 vol = 0x40000000;
	AXSetVoiceVe(voice1.voice, &vol);

	//Device mix? Volume of DRC/TV?
	u32 mix[24];
	memset(mix, 0, sizeof(mix));
	mix[0] = vol;
	mix[4] = vol;
	
	AXSetVoiceDeviceMix(voice1.voice, 0, 0, mix);
	AXSetVoiceDeviceMix(voice1.voice, 1, 0, mix);
	
	AXVoiceEnd(voice1.voice);
	
	/*!
		Generate Crash Tone (D at offset 0 with sawtooth wave)
	*/
	
	// D
	voice1.newFrequency = (float)(440 * exp((-5 + 0) * log(2)/12));
	//wipe the existing samples
	memset(voice1.samples, 0, SAMPLE_BUFFER_MAX_SIZE);
	//generate new samples
	voice1.numSamples = generateSawtoothWave(voice1.samples, SAMPLE_BUFFER_MAX_SIZE, voice1.newFrequency);
	//flush the samples to memory
	DCFlushRange(voice1.samples, SAMPLE_BUFFER_MAX_SIZE);
	//mark the voice as modified so the audio callback will reload it
	voice1.modified = 1;
	//flush changes to memory
	DCFlushRange(&voice1, sizeof(voice1));
	
	for(;;);
	// Well, there's no reason to deinitialize the libraries at this point...
}