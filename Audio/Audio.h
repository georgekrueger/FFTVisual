/*
 *  SamplePlayer.h
 *  FFTVisual
 *
 *  Created by George Krueger on 2/26/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef AUDIO_H
#define AUDIO_H

#include "AudioDefines.h"

struct AudioHandle
{
	AudioSample* Buffer;
	uint         NumChannels;
	uint         NumFrames;
};

bool Audio_Init(uint SampleRate, uint FrameBufferSize);
bool Audio_Deinit();
bool Audio_LoadFile(const char* InAudioFile, AudioHandle* OutHandle);
bool Audio_ReleaseHandle(AudioHandle* Handle);
bool Audio_PlayHandle(const AudioHandle* Handle, float Volume, bool Loop);

#endif