/*	MikMod sound library
	(c) 1998, 1999, 2000 Miodrag Vallat and others - see file AUTHORS for
	complete list.

	This library is free software; you can redistribute it and/or modify
	it under the terms of the GNU Library General Public License as
	published by the Free Software Foundation; either version 2 of
	the License, or (at your option) any later version.
 
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Library General Public License for more details.
 
	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
	02111-1307, USA.
*/

/*==============================================================================

  $Id: drv_raw.c,v 1.2 2004/01/21 17:29:32 raph Exp $

  Driver for output to OpenSL ES

==============================================================================*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include <assert.h>
// for native audio
#include <SLES/OpenSLES.h>
#include "SLES/OpenSLES_Android.h"

#include "mikmod_internals.h"
#include "logging.h"

#define NUMBUFFERS	2				/* number of buffers */
#define BUFFERSIZE	120				/* buffer size in milliseconds */

#if defined(WIN32) && !defined(__MWERKS__)
#define open _open
#endif

// engine interfaces
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;

// output mix interfaces
static SLObjectItf outputMixObject = NULL;
static SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;

// buffer queue player interfaces
static SLObjectItf bqPlayerObject = NULL;
static SLPlayItf bqPlayerPlay;
static SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
static SLEffectSendItf bqPlayerEffectSend;

// aux effect on the output mix, used by the buffer queue player
static const SLEnvironmentalReverbSettings reverbSettings =
    SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;


signed char*		buffer[NUMBUFFERS];		/* pointers to buffers */
short				buffersout;				/* number of buffers playing/about to be played */
short				nextbuffer;				/* next buffer to be mixed */
unsigned long		buffersize;				/* buffer size in bytes */


// this callback handler is called every time a buffer finishes playing
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    assert(bq == bqPlayerBufferQueue);
    assert(NULL == context);
    --buffersout;
}

// create buffer queue audio player
void createBufferQueueAudioPlayer()
{
	SLuint32 sample_rate = md_mixfreq * 1000;//SL_SAMPLINGRATE_44_1;
	SLresult result;
	int channels = 1;

	if (md_mode&DMODE_STEREO)
		channels = 2;

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, channels, sample_rate,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // create audio player
    const SLInterfaceID ids[2] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND};
    const SLboolean req[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc, &audioSnk,
            2, ids, req);
    assert(SL_RESULT_SUCCESS == result);

    // realize the player
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

    // get the play interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    assert(SL_RESULT_SUCCESS == result);

    // get the buffer queue interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
            &bqPlayerBufferQueue);
    assert(SL_RESULT_SUCCESS == result);

    // register callback on the buffer queue
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, NULL);
    assert(SL_RESULT_SUCCESS == result);

    // get the effect send interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_EFFECTSEND,
            &bqPlayerEffectSend);
    assert(SL_RESULT_SUCCESS == result);

    // set the player's state to playing
    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);
}

static BOOL OSLES_IsPresent(void)
{
	return 1;
}

static BOOL OSLES_Init(void)
{
	short			samplesize;
	int				n;

	samplesize=1;
	if (md_mode&DMODE_STEREO) samplesize<<=1;
	if (md_mode&DMODE_16BITS) samplesize<<=1;

	SLresult result;

    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if(SL_RESULT_SUCCESS != result)
    	return 1;

    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if(SL_RESULT_SUCCESS != result)
    	return 1;

    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    if(SL_RESULT_SUCCESS != result)
    	return 1;

    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, ids, req);
    if(SL_RESULT_SUCCESS != result)
    	return 1;

    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if(SL_RESULT_SUCCESS != result)
    	return 1;

    /*
    // get the environmental reverb interface
    // this could fail if the environmental reverb effect is not available,
    // either because the feature is not present, excessive CPU load, or
    // the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
            &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
    }
*/
    createBufferQueueAudioPlayer();
    buffersize=md_mixfreq*samplesize*BUFFERSIZE/1000;

	for (n=0;n<NUMBUFFERS;n++) {
		buffer[n]=_mm_malloc(buffersize);
		if (!buffer[n]) {
			_mm_errno=MMERR_OUT_OF_MEMORY;
			return 1;
		}
	}

	md_mode|=DMODE_SOFT_MUSIC|DMODE_SOFT_SNDFX;
	buffersout=nextbuffer=0;

	return VC_Init();
}

static void OSLES_Exit(void)
{
	int n;

	VC_Exit();
	for (n=0;n<NUMBUFFERS;n++) {
		_mm_free(buffer[n]);
	}

    // destroy buffer queue audio player object, and invalidate all associated interfaces
    if (bqPlayerObject != NULL) {
    	SLuint32 state;
    	(*bqPlayerPlay)->GetPlayState(bqPlayerPlay, &state);
    	if (state == SL_PLAYSTATE_PLAYING)
    		(*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_STOPPED);
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = NULL;
        bqPlayerPlay = NULL;
        bqPlayerBufferQueue = NULL;
        bqPlayerEffectSend = NULL;
    }

    // destroy output mix object, and invalidate all associated interfaces
    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }

    // destroy engine object, and invalidate all associated interfaces
    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }
}

static void OSLES_Update(void)
{
	unsigned long done = 1;

	while (buffersout < NUMBUFFERS) {
		done=VC_WriteBytes(buffer[nextbuffer],buffersize);
		if (!done) break;
        SLresult result;
        // enqueue another buffer
        result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, buffer[nextbuffer], done);
        if(SL_RESULT_SUCCESS != result)
        	break;
		if (++nextbuffer>=NUMBUFFERS) nextbuffer%=NUMBUFFERS;
		++buffersout;
	}
}

static void OSLES_Pause(void)
{
	SLuint32 state;
/*
	if (NULL == bqPlayerPlay)
		return;

	(*bqPlayerPlay)->GetPlayState(bqPlayerPlay, &state);
	if (state == SL_PLAYSTATE_PLAYING)
		(*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PAUSED);
	else if (state == SL_PLAYSTATE_PAUSED)
		(*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
*/
}

static BOOL OSLES_Reset(void)
{

	return 0;
}

MIKMODAPI MDRIVER drv_osles={
	NULL,
	"Android OpenSL ES Driver",
	"Android Native Audio OpenSL ES Output v1.0",
	0,255,
	"osles",

	NULL,
	OSLES_IsPresent,
	VC_SampleLoad,
	VC_SampleUnload,
	VC_SampleSpace,
	VC_SampleLength,
	OSLES_Init,
	OSLES_Exit,
	OSLES_Reset,
	VC_SetNumVoices,
	VC_PlayStart,
	VC_PlayStop,
	OSLES_Update,
	OSLES_Pause,
	VC_VoiceSetVolume,
	VC_VoiceGetVolume,
	VC_VoiceSetFrequency,
	VC_VoiceGetFrequency,
	VC_VoiceSetPanning,
	VC_VoiceGetPanning,
	VC_VoicePlay,
	VC_VoiceStop,
	VC_VoiceStopped,
	VC_VoiceGetPosition,
	VC_VoiceRealVolume
};

/* ex:set ts=4: */
