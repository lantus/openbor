/*
 * OpenBOR - http://www.LavaLit.com
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in OpenBOR root for details.
 *
 * Copyright (c) 2004 - 2011 OpenBOR Team
 */

// A soundblaster interface. No bugs?

#include <dos/dostags.h>
#include <hardware/cia.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/ahi.h>
#include <proto/graphics.h>
 

#include "sblaster.h"
#include "soundmix.h"
 
#define USE_AHI_V4 TRUE

#define CHANNELS   2
#define MAXSAMPLES 16

#define INT_FREQ   50

#define MIXER_MAX_CHANNELS 16
#define BUFFER_LEN 16384

extern int throttle;
static int audio_sample_rate;
unsigned char *play_buffer = NULL;

#define NB_SAMPLES 128 /* better resolution */  
 

static void sub_invoc(void);	// Sound sub-process
void sub_func(void);
struct Process *sound_process;
int quit_sig, pause_sig,
	resume_sig, ahi_sig;		// Sub-process signals
struct Task *main_task;			// Main task
int main_sig;					// Main task signals
static ULONG sound_func(void);	// AHI callback
struct MsgPort *ahi_port;		// Port and IORequest for AHI
struct AHIRequest *ahi_io;
struct AHIAudioCtrl *ahi_ctrl;	// AHI control structure
struct AHISampleInfo sample;	// SampleInfos for double buffering
struct Hook sf_hook;			// Hook for callback function
int play_buf;					// Number of buffer currently playing
bool ready;                     // Is the audio ready?
	
// Library bases
struct Library *AHIBase;

// CIA-A base
extern struct CIA ciaa;  
 
static int started;

int SB_playstart(int bits, int samplerate) {
	
    (unsigned char *)play_buffer = (unsigned char *)malloc(4096); 
    // Find our (main) task
	main_task = FindTask(NULL);

	// Create signal for communication
	main_sig = AllocSignal(-1);

	// Create sub-process and wait until it is ready
	if ((sound_process = CreateNewProcTags(
		NP_Entry, (ULONG)&sub_invoc,
		NP_Name, (ULONG)"SoundProcess",
		NP_Priority, 1, 
		TAG_DONE)) != NULL)
		Wait(1 << main_sig); 
        	 
	return 1;
}

void SB_playstop() {
	 
    // Tell sub-process to quit and wait for completion
	if (sound_process != NULL) {
		Signal(&(sound_process->pr_Task), 1 << quit_sig);
		Wait(1 << main_sig);
	}

	// Free signal
	FreeSignal(main_sig); 
    	 
}

void sub_invoc(void)
{	 
    sub_func();
}

ULONG sound_func(void)
{
	register struct AHIAudioCtrl *ahi_ctrl asm ("a2");	 
    update_sample(play_buffer, 4096);	
	AHI_SetSound(0, play_buf, 0, 0, ahi_ctrl, 0);
	Signal(&(sound_process->pr_Task), 1 << (ahi_sig));
	return 0;
}
 
 
void sub_func(void)
{
	ahi_port = NULL;
	ahi_io = NULL;
	ahi_ctrl = NULL;
	sample.ahisi_Address = NULL;
	ready = FALSE;

	// Create signals for communication
	quit_sig = AllocSignal(-1);
	pause_sig = AllocSignal(-1);
	resume_sig = AllocSignal(-1);
	ahi_sig = AllocSignal(-1);

	// Open AHI
	if ((ahi_port = CreateMsgPort()) == NULL)
		goto wait_for_quit;
	if ((ahi_io = (struct AHIRequest *)CreateIORequest(ahi_port, sizeof(struct AHIRequest))) == NULL)
		goto wait_for_quit;
	ahi_io->ahir_Version = 2;
	if (OpenDevice(AHINAME, AHI_NO_UNIT, (struct IORequest *)ahi_io, NULL))
		goto wait_for_quit;
	AHIBase = (struct Library *)ahi_io->ahir_Std.io_Device;

	// Initialize callback hook
	sf_hook.h_Entry = sound_func;

	// Open audio control structure
	if ((ahi_ctrl = AHI_AllocAudio(
		AHIA_AudioID, 0x0002000b,
		AHIA_MixFreq, 22050,
		AHIA_Channels, 2,
		AHIA_Sounds, 1,
		AHIA_SoundFunc, (ULONG)&sf_hook,
		TAG_DONE)) == NULL)
		goto wait_for_quit;

 
	
    // 32 bits (4 bytes) are required per sample for storage (16bit stereo).
    ULONG sampleBufferSize = (2 * AHI_SampleFrameSize(AHIST_M16S));
  
	// Prepare SampleInfos and load sounds (two sounds for double buffering)
	sample.ahisi_Type = AHIST_M16S;
	sample.ahisi_Length = sampleBufferSize;
	sample.ahisi_Address = AllocVec(sampleBufferSize, MEMF_PUBLIC | MEMF_CLEAR);
 
	if (sample.ahisi_Address == NULL)
		goto wait_for_quit;
	AHI_LoadSound(0, AHIST_DYNAMICSAMPLE, &sample, ahi_ctrl);
 
	// Set parameters
	play_buf = 0;
	AHI_SetVol(0, 0x10000, 0x8000, ahi_ctrl, AHISF_IMM);
	AHI_SetFreq(0, 22050 , ahi_ctrl, AHISF_IMM);
	AHI_SetSound(0, play_buf, 0, 0, ahi_ctrl, AHISF_IMM);

	// Start audio output
	AHI_ControlAudio(ahi_ctrl, AHIC_Play, TRUE, TAG_DONE);

	// We are now ready for commands
	ready = TRUE;
	Signal(main_task, 1 << main_sig);

	// Accept and execute commands
	for (;;) {
		ULONG sigs = Wait((1 << quit_sig) | (1 << pause_sig) | (1 << resume_sig) | (1 << ahi_sig));

		// Quit sub-process
		if (sigs & (1 << quit_sig))
			goto quit;

		// Pause sound output
		if (sigs & (1 << pause_sig))
			AHI_ControlAudio(ahi_ctrl, AHIC_Play, FALSE, TAG_DONE);

		// Resume sound output
		if (sigs & (1 << resume_sig))
			AHI_ControlAudio(ahi_ctrl, AHIC_Play, TRUE, TAG_DONE);

		// Calculate next buffer
		if (sigs & (1 << ahi_sig))
		{
		//	memcpy((unsigned char *)sample.ahisi_Address, play_buffer, 128);
        }
	}

wait_for_quit:
	// Initialization failed, wait for quit signal
	Wait(1 << quit_sig);

quit:
	// Free everything
	if (ahi_ctrl != NULL) {
		AHI_ControlAudio(ahi_ctrl, AHIC_Play, FALSE, TAG_DONE);
		AHI_FreeAudio(ahi_ctrl);
		CloseDevice((struct IORequest *)ahi_io);
	}

	FreeVec(sample.ahisi_Address);
 

	if (ahi_io != NULL)
		DeleteIORequest((struct IORequest *)ahi_io);

	if (ahi_port != NULL)
		DeleteMsgPort(ahi_port);

	FreeSignal(quit_sig);
	FreeSignal(pause_sig);
	FreeSignal(resume_sig);
	FreeSignal(ahi_sig);

	// Quit (synchronized with main task)
	Forbid();
	Signal(main_task, 1 << main_sig); 
	
}

void SB_setvolume(char dev, char volume) {
	 
}


void SB_updatevolume(int volume) {
 
}
