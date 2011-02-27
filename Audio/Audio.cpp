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

pthread_mutex_t AudioMutex;

struct AudioPlayHead
{
	bool               Active;
	const AudioHandle* Handle;
	float              Volume;
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
    outputParameters.channelCount = 2;       /* stereo output */
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

bool Audio_PlayHandle(const AudioHandle* Handle, float Volume)
{
	// find the first open spot to add play head
	pthread_mutex_lock(&AudioMutex);
	bool Added = false;
	for (uint i=0; i<AUDIO_MAX_SIMULTANEOUS_PLAYS; i++) {
		if (!AudioData.PlayHeads[i].Active) {
			AudioData.PlayHeads[i].Active = true;
			AudioData.PlayHeads[i].Handle = Handle;
			AudioData.PlayHeads[i].Volume = Volume;
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
	uint NumSamplesInFile = PlayHead->Handle->NumFrames * PlayHead->Handle->NumChannels;
	uint NumSamplesLeft = NumSamplesInFile - PlayHead->Position;
	uint NumSamplesToProcess = MIN(NumFrames, NumSamplesLeft);
	
	for (uint i=0; i<NumSamplesToProcess; i++) {
		*Out = PlayHead->Handle->Buffer[PlayHead->Position] * PlayHead->Volume;
		Out++;
		PlayHead->Position++;
	}
	
	return ( NumSamplesLeft <= NumFrames ? DONE : RENDERING );
}

static int PortAudioCallback( const void *inputBuffer, void *outputBuffer,
							 unsigned long framesPerBuffer,
							 const PaStreamCallbackTimeInfo* timeInfo,
							 PaStreamCallbackFlags statusFlags,
							 void *userData )
{
	// clear output buffer
	memset(outputBuffer, 0, sizeof(AudioSample) * framesPerBuffer);
	
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

