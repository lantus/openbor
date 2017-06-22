/*
 * OpenBOR - http://www.LavaLit.com
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in OpenBOR root for details.
 *
 * Copyright (c) 2004 - 2011 OpenBOR Team
 */

// A soundblaster interface. No bugs?

#include <exec/exec.h>
#include <exec/io.h>
#include <dos/dostags.h>
#include <hardware/cia.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/ahi.h>
#include <proto/graphics.h>
#include "amigaport.h"
#include "sblaster.h"
#include "soundmix.h"
 
#define USE_AHI_V4 TRUE


#define SAMPLES_PER_SEC 44100

#define PLOG(fmt, args...) do {   fprintf(stdout, fmt, ## args); } while (0)

static struct MsgPort *ahiPort = NULL;;
static struct AHIRequest *ahiReq[2] = { NULL, NULL };
static bool ahiReqSent[2] = { false, FALSE };

static BYTE *soundBuffer[2]	 = { NULL, NULL };
static BYTE _currentSoundBuffer = 0;

static uint32_t  _mixingFrequency = 0;
static uint32_t  _sampleCount = 0;
static uint32_t  _sampleBufferSize = 0;
 
static struct Task *g_soundThread = NULL;
	
// Library bases
struct Library *AHIBase;

static BYTE audioRunning = 0;
extern int quit_game;

int init_sound() {



    ahiPort = (struct MsgPort *)CreateMsgPort();    
    ahiReq[0] = (struct AHIRequest *)CreateIORequest(ahiPort, sizeof(struct AHIRequest));
    
    // Open at least version 4.
    ahiReq[0]->ahir_Version = 4;
    
    BYTE deviceError = OpenDevice(AHINAME, AHI_DEFAULT_UNIT, (struct IORequest*)ahiReq[0], 0);
    
    if (deviceError == 0)
    {
        // 32 bits (4 bytes) are required per sample for storage (16bit stereo).
        _sampleBufferSize = (_sampleCount * 2);
        
        soundBuffer[0] = (BYTE *)AllocVec(_sampleBufferSize, MEMF_PUBLIC | MEMF_CLEAR);
    	soundBuffer[1] = (BYTE *)AllocVec(_sampleBufferSize, MEMF_PUBLIC | MEMF_CLEAR);
    	 
    
        // Make a copy of the request (for double buffering)
        ahiReq[1] = (struct AHIRequest *)AllocVec(sizeof(struct AHIRequest), MEMF_PUBLIC);
        
    
    
    	CopyMem(ahiReq[0], ahiReq[1], sizeof(struct AHIRequest));
    
    
    	_currentSoundBuffer = 0;
    	ahiReqSent[0] = false;
    	ahiReqSent[1] = false;
    	
    	audioRunning = 1;
    }
    else
    {
        PLOG("\nCannot Initialize Audio : Please install AHI V4.X or Higher\n");
        PLOG("*************************************************************\n\n");
        quit_game = 1;
        
    }
     
}

void exit_sound() {

    
	if (ahiReq[1]) {
		FreeVec(ahiReq[1]);
		ahiReq[1] = NULL;
    }

	if (ahiReq[0]) {
		CloseDevice((struct IORequest *) ahiReq[0]);
		DeleteIORequest(ahiReq[0]);
		ahiReq[0] = NULL;
	}

	if (soundBuffer[0])
	{
		FreeVec((APTR) soundBuffer[0]);
		soundBuffer[0] = NULL;
    }

	if (soundBuffer[1])
	{
		FreeVec((APTR) soundBuffer[1]);
		soundBuffer[1] = NULL;
    }

	if (ahiPort)
	{
		DeleteMsgPort(ahiPort);
		ahiPort = NULL;
    }
}

int sound_thread(STRPTR args, ULONG length) {
    
    LONG priority = 0;
    ULONG signals;

    init_sound();


    for (;;) {
        
        if (audioRunning)
        {
    		while (!ahiReqSent[_currentSoundBuffer] || CheckIO((struct IORequest *) ahiReq[_currentSoundBuffer])) {
    
                if (ahiReqSent[_currentSoundBuffer]) {
    				WaitIO((struct IORequest *) ahiReq[_currentSoundBuffer]);
                }
    			            
        		ahiReq[_currentSoundBuffer]->ahir_Std.io_Message.mn_Node.ln_Pri = priority;
        		ahiReq[_currentSoundBuffer]->ahir_Std.io_Command = CMD_WRITE;
        		ahiReq[_currentSoundBuffer]->ahir_Std.io_Data    = soundBuffer[_currentSoundBuffer];
        		ahiReq[_currentSoundBuffer]->ahir_Std.io_Length  = _sampleBufferSize;
        		ahiReq[_currentSoundBuffer]->ahir_Std.io_Offset  = 0;
        		ahiReq[_currentSoundBuffer]->ahir_Type		     = AHIST_S16S;
        		ahiReq[_currentSoundBuffer]->ahir_Frequency		 = _mixingFrequency;
        		ahiReq[_currentSoundBuffer]->ahir_Position       = 0x8000;
        		ahiReq[_currentSoundBuffer]->ahir_Volume		 = 0x10000;
                ahiReq[_currentSoundBuffer]->ahir_Link		     = (ahiReqSent[_currentSoundBuffer^1]) ? ahiReq[_currentSoundBuffer^1] : NULL;
                
                   
                update_sample((BYTE *)soundBuffer[_currentSoundBuffer], _sampleBufferSize);              
                    
                SendIO((struct IORequest *)ahiReq[_currentSoundBuffer]);
        
        		ahiReqSent[_currentSoundBuffer] = true;  
        
                // Flip.
                _currentSoundBuffer ^= 1;
                
            }
    		
        }
		signals = Wait(SIGBREAKF_CTRL_C | (1 << ahiPort->mp_SigBit));
		if (signals & SIGBREAKF_CTRL_C) {
			break;
        }
    }
    
    

	if (ahiReqSent[_currentSoundBuffer]) {
		AbortIO((struct IORequest *) ahiReq[_currentSoundBuffer]);
		WaitIO((struct IORequest *) ahiReq[_currentSoundBuffer]);
	}

	if (ahiReqSent[_currentSoundBuffer^1]) {
		AbortIO((struct IORequest *) ahiReq[!_currentSoundBuffer]);
		WaitIO((struct IORequest *) ahiReq[!_currentSoundBuffer]);
	}

    audioRunning = 0;
    exit_sound();

	return 0;
}


 
int SB_playstart(int bits, int samplerate) {

    if (_mixingFrequency == 0) {
		_mixingFrequency = SAMPLES_PER_SEC;
    } 

	// Determine the sample buffer size. We want it to store enough data for
	// at least 1/16th of a second (though at most 8192 samples). Note
	// that it must be a power of two. So e.g. at 22050 Hz, we request a
	// sample buffer size of 2048.
	_sampleCount = 2048;
	while ((_sampleCount * 16) > (_mixingFrequency * 2)) {
		_sampleCount >>= 1;
    }
      	
         
    // Create the mixer instance and start the sound processing.
    

    g_soundThread = (struct Task *)CreateNewProcTags(
					NP_Name, (ULONG)"MixerThread",
					NP_CloseOutput, FALSE,
                    NP_CloseInput, FALSE,
					NP_StackSize, 20000,
					NP_Entry, (ULONG)&sound_thread,
					TAG_DONE);
					
  
	
    SetTaskPri(g_soundThread, 2);

    return 1;
    
}


void SB_playstop() {
 
    if (g_soundThread) {
        Signal(g_soundThread, SIGBREAKF_CTRL_C);
    	Delay(10);
        g_soundThread = NULL;
    }
        	 
}
 
void SB_setvolume(char dev, char volume) {
	 
}


void SB_updatevolume(int volume) {
 
} 
