
#include <exec/types.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <exec/exec.h>
#include <dos/dos.h>
#include <devices/gameport.h>
#include <devices/inputevent.h>
#include <devices/input.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/layers.h>
#include <proto/intuition.h>
#include <proto/asl.h>
#include <proto/timer.h>
#include <proto/keymap.h>
#include <proto/lowlevel.h>

#include "video.h"
#include "globals.h"
#include "stristr.h"
#include "sblaster.h"
#include "joysticks.h"
#include "control.h"
 
#define JOY_X_DELTA (1)
#define JOY_Y_DELTA (1)
#define TIMEOUT_SECONDS (10)

#define REG(xn, parm) parm __asm(#xn)
#define REGARGS __regargs
#define STDARGS __stdargs
#define SAVEDS __saveds
#define ALIGNED __attribute__ ((aligned(4))
#define FAR
#define CHIP
#define INLINE __inline__
 
struct Library *LowLevelBase=NULL;
extern struct Window *_hardwareWindow;
extern struct ExecBase *SysBase;
extern int quit_game;
unsigned long getPad();

static unsigned long lastkey = 0;                                
/*
Reset All data back to Zero and
destroy all SDL Joystick data.
*/
void control_exit() {

    if (LowLevelBase)
    {
        CloseLibrary(LowLevelBase);
        LowLevelBase = NULL;
    }
}


/*
Create default values for joysticks if enabled.
Then scan for joysticks and update their data.
*/
void control_init(int joy_enable) {
 
     if(!LowLevelBase) LowLevelBase = (struct Library *) OpenLibrary((UBYTE *)"lowlevel.library",0);
    
   
}

void control_rumble(int port, int msec) {
}

/*
Set global variable, which is used for
enabling and disabling all joysticks.
*/
int control_usejoy(int enable) {
   
	return 0;
}


/*
Only used in openbor.c to get current status
of joystick usage.
*/
int control_getjoyenabled() {
	return 1;
}


/*
Convert binary masked data to indexes
*/
static int flag_to_index(u32 flag) {
    char index = 0;
    while (flag >>= 1) index ++;
	   return index; 
}

void control_setkey(s_playercontrols * pcontrols, unsigned int flag, int key) {
	if(!pcontrols) return;
	pcontrols->settings[flag_to_index(flag)] = key;
	pcontrols->keyflags = pcontrols->newkeyflags = 0; 
}

int keyboard_getlastkey() {
     
}

 

int control_scankey() {
 
    return (getPad() & 0x3ff);    
}


#define KEY_RETURN (0x44)
#define KEY_F10    (0x59)
 
unsigned long getKey()
{
    struct IntuiMessage *imsg;
    ULONG imsg_Class;
    UWORD Code;
    int ret = 0;
   
    if(imsg = (struct IntuiMessage*)GetMsg(_hardwareWindow->UserPort))
    {
    	imsg_Class = imsg->Class;
    	Code  = imsg->Code;
    	ReplyMsg((struct Message*)imsg);
    	switch(imsg_Class)
    	{ 
    	case IDCMP_RAWKEY			:
    		if(Code & 0x80)
    		{
    			switch(Code & 0x7f)
    			{    
    				case KEY_RETURN:		ret =  0x4000;
    				    break;  
                    case KEY_F10:
                        quit_game = 1;
                        break;      			     				 
    			}
    		} else
    		{
    			switch(Code)
    			{
 
    			}
    		}
        }
    }                    
    
    return ret;
}
unsigned long getPad()
{
    ULONG data = 0;
	unsigned long btns=0;

    if (LowLevelBase != NULL) {
    	data = ReadJoyPort(1);	
    	
    	switch (data & JP_TYPE_MASK)
    	{
            case JP_TYPE_JOYSTK:

                if(data&JPF_JOY_UP)
                    btns |= JPF_JOY_UP;


                if(data&JPF_JOY_RIGHT)
                    btns |= JPF_JOY_RIGHT;

                if(data&JPF_JOY_DOWN)
                    btns |= JPF_JOY_DOWN;


                if(data&JPF_JOY_LEFT)
                    btns |= JPF_JOY_LEFT;


                if(data&JPF_BUTTON_RED)
                {
                    btns |= JPF_BUTTON_RED;
                    PDEBUG("Fire 1 Pressed\n");
                }


                if(data&JPF_BUTTON_BLUE)
                {
                    btns |= JPF_BUTTON_BLUE;
                    PDEBUG("Fire 2 Pressed\n");
                }
                
                if ((data&JPF_BUTTON_BLUE) && (data&JPF_BUTTON_RED))
                {
                    btns |= 0x400;
                }
                
                                    
                break;
            case JP_TYPE_GAMECTLR:          // CD32 pad
            
                if(data&JPF_JOY_UP)
                    btns |= JPF_JOY_UP;


                if(data&JPF_JOY_RIGHT)
                    btns |= JPF_JOY_RIGHT;

                if(data&JPF_JOY_DOWN)
                    btns |= JPF_JOY_DOWN;


                if(data&JPF_JOY_LEFT)
                    btns |= JPF_JOY_LEFT;


                if(data&JPF_BUTTON_RED)
                {
                    btns |= JPF_BUTTON_RED;
                    PDEBUG("Fire 1 Pressed\n");
                }


                if(data&JPF_BUTTON_BLUE)
                {
                    btns |= JPF_BUTTON_BLUE;
                    PDEBUG("Fire 2 Pressed\n");
                }
                
                if (data&JPF_BUTTON_YELLOW)
                {
                    btns |= 0x400;
                }                
                            
                break;
        }
    }
DONE:
  	return lastkey = btns;
}

void control_update(s_playercontrols ** playercontrols, int numplayers) {
   // control routines
	unsigned long k = 0;
    s_playercontrols * pcontrols = playercontrols[0];
    unsigned padbits = getPad();
    unsigned keybits = getKey();
    int i;
    
    if (quit_game)
        return;

    for (i = 1; i < 9; i ++)
    {    
      if (padbits & pcontrols->settings[i])
         k |= (1 << i);
      if (keybits & pcontrols->settings[i])
         k |= (1 << i);         
         
    }

    pcontrols->kb_break = 0;
    pcontrols->newkeyflags = k & (~pcontrols->keyflags);
    pcontrols->keyflags = k;  
}


char *control_getkeyname(unsigned int keycode) {
	static char str[256];

   switch (keycode)
   {
      case 0x100: return "Start";
      case 0x001: return "Left";
      case 0x004: return "Right";
      case 0x008: return "Up";
      case 0x002: return "Down";
      case 0x080: return "Right paddle";
      case 0x040: return "'A' Button";
      case 0x020: return "'B' Button";
      case 0x010: return "Left paddle";
      case 0x200: return "Select";
      default: return "Unknown";
    }
}
 
