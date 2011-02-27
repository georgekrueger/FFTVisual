/*
 *  Audio System
 *
 *  Created by George Krueger on 2/26/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../../portaudio/include/portaudio.h"
#include "wavfile.h"
#include "AudioDefines.h"
#include "Audio.h"
#include "pthread.h"

#define MIN(a, b) ( (a) < (b) ? a : b )

static const uint AUDIO_MAX_SIMULTANEOUS_PLAYS = 10; // max number of audio files that can be played at a time
static const uint AUDIO_NUM_OUTPUT_CHANNELS = 2;

pthread_mutex_t AudioMutex;

struct AudioPlayHead
{
	bool               Active;
	const AudioHandle* Handle;
	float              Volume;
	bool               Loop;
	uint               Position;
};

enum AudioRenderCode
{
	RENDERING,
	DONE
};

static struct {
	
	AudioPlayHead PlayHeads[AUDIO_MAX_SIMULTANEOUS_PLAYS];
	
	uint SampleRate;
	uint FrameBufferSize;
	
	PaStream* Stream;
	
} AudioData;

// Portaudio callback
static int PortAudioCallback( const void *inputBuffer, void *outputBuffer,
							 unsigned long framesPerBuffer,
							 const PaStreamCallbackTimeInfo* timeInfo,
							 PaStreamCallbackFlags statusFlags,
							 void *userData );

bool Audio_Init(uint SampleRate, uint FrameBufferSize)
{	
	// Initialize Portaudio
	PaStreamParameters outputParameters;
	PaError err = Pa_Initialize();
	if (err != paNoError) { assert(false); return false; }
	
	outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    outputParameters.channelCount = AUDIO_NUM_OUTPUT_CHANNELS;       /* stereo output */
    outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;
	
	err = Pa_OpenStream(&AudioData.Stream, NULL, &outputParameters, SampleRate, FrameBufferSize, 0, PortAudioCallback, NULL );
	if (err != paNoError) { assert(false); return false; }
	
	err = Pa_StartStream( AudioData.Stream );
	if (err != paNoError) { assert(false); return false; }
	
	for (uint i=0; i<AUDIO_MAX_SIMULTANEOUS_PLAYS; i++) {
		AudioData.PlayHeads[i].Active = false;
	}
	
	AudioData.SampleRate = SampleRate;
	AudioData.FrameBufferSize = FrameBufferSize;
	
	return true;
}

bool Audio_Deinit()
{
	// Cleanup Portaudio
	PaError err = Pa_StopStream( AudioData.Stream );
    if (err != paNoError) { assert(false); return false; }
	
    err = Pa_CloseStream( AudioData.Stream );
    if (err != paNoError) { assert(false); return false; }
	
    Pa_Terminate();
	
	return true;
}

bool Audio_LoadFile(const char* InAudioFile, AudioHandle* OutHandle)
{
	bool ReadSuccess = ReadWavFile(InAudioFile, &OutHandle->Buffer, &OutHandle->NumFrames, &OutHandle->NumChannels);
	if (!ReadSuccess) return false;
	
	return true;
}

bool Audio_ReleaseHandle(AudioHandle* Handle)
{
	free(Handle->Buffer);
	return true;
}

bool Audio_PlayHandle(const AudioHandle* Handle, float Volume, bool Loop)
{
	// find the first open spot to add play head
	pthread_mutex_lock(&AudioMutex);
	bool Added = false;
	for (uint i=0; i<AUDIO_MAX_SIMULTANEOUS_PLAYS; i++) {
		if (!AudioData.PlayHeads[i].Active) {
			AudioData.PlayHeads[i].Active = true;
			AudioData.PlayHeads[i].Handle = Handle;
			AudioData.PlayHeads[i].Volume = Volume;
			AudioData.PlayHeads[i].Loop = Loop;
			AudioData.PlayHeads[i].Position = 0;
			Added = true;
			break;
		}
	}
	pthread_mutex_unlock(&AudioMutex);
	
	if (!Added) assert(false); return false;
	
	return true;
}

// Render the requested number of frames to OutBuffer.  If
static AudioRenderCode RenderPlayHead(AudioPlayHead* PlayHead, AudioSample* OutBuffer, uint NumFrames)
{
	AudioSample *Out = OutBuffer;
	uint NumFramesLeftInHandle = PlayHead->Handle->NumFrames - ( PlayHead->Position / PlayHead->Handle->NumChannels);
	uint NumFramesToProcess = (PlayHead->Loop ? NumFrames : MIN(NumFrames, NumFramesLeftInHandle));
	uint NumSamplesInHandle = PlayHead->Handle->NumFrames * PlayHead->Handle->NumChannels;
	
	if (AUDIO_NUM_OUTPUT_CHANNELS == 2) {
		if (PlayHead->Handle->NumChannels == 1) {
			for (uint i=0; i<NumFramesToProcess; i++) {
				AudioSample Sample = PlayHead->Handle->Buffer[PlayHead->Position] * PlayHead->Volume;
				Out[0] += Sample;
				Out[1] += Sample;
				Out += 2;
				PlayHead->Position++;
				if (PlayHead->Position >= NumSamplesInHandle) PlayHead->Position = 0; // wrap around play head
			}
		}
		else if (PlayHead->Handle->NumChannels == 2) {
			for (uint i=0; i<NumFramesToProcess; i++) {
				Out[0] += PlayHead->Handle->Buffer[PlayHead->Position] * PlayHead->Volume;
				PlayHead->Position++;
				if (PlayHead->Position >= NumSamplesInHandle) PlayHead->Position = 0; // wrap around play head
				
				Out[1] += PlayHead->Handle->Buffer[PlayHead->Position] * PlayHead->Volume;
				PlayHead->Position++;
				if (PlayHead->Position >= NumSamplesInHandle) PlayHead->Position = 0; // wrap around play head
				
				Out += 2;
			}
		}
	}
	else {
		assert(false); // mono outut not supported
	}
	
	return ( !PlayHead->Loop && NumFramesLeftInHandle <= NumFrames ? DONE : RENDERING );
}

static int PortAudioCallback( const void *inputBuffer, void *outputBuffer,
							 unsigned long framesPerBuffer,
							 const PaStreamCallbackTimeInfo* timeInfo,
							 PaStreamCallbackFlags statusFlags,
							 void *userData )
{
	// clear output buffer
	memset(outputBuffer, 0, sizeof(AudioSample) * framesPerBuffer * AUDIO_NUM_OUTPUT_CHANNELS);
	
	pthread_mutex_lock(&AudioMutex);
	
	// update play heads
	for (uint i=0; i<AUDIO_MAX_SIMULTANEOUS_PLAYS; i++) {
		AudioPlayHead* PlayHead = &AudioData.PlayHeads[i];
		if (PlayHead->Active) {
			AudioRenderCode RenderCode = RenderPlayHead(PlayHead, (AudioSample*)outputBuffer, framesPerBuffer);
			if (RenderCode == DONE) {
				PlayHead->Active = false;
			}
		}
	}
	
	pthread_mutex_unlock(&AudioMutex);
	
	return paContinue;
}

