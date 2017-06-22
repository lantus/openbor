/*
 * OpenBOR - http://www.LavaLit.com
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in OpenBOR root for details.
 *
 * Copyright (c) 2004 - 2011 OpenBOR Team
 */
#include <exec/exec.h>
#include "amigaport.h"
#include "packfile.h"
#include "video.h"
#include "menu.h"

#define PLOG(fmt, args...) do {   fprintf(stdout, fmt, ## args); } while (0)
#define appExit exit
#undef exit
extern struct ExecBase *SysBase;

int isRTG;
int cpu_type;
char packfile[128] = { "PAKS/bor/" };
char savesDir[128] = { "Saves" };
char logsDir[128] = { "Logs" };
char screenShotsDir[128] = { "ScreenShots" };

void borExit(int reset) {
    video_end();    
    exit(0);    
}

extern  int kprintf (char *fmt, ... );

int kprintf (char *fmt, ... )
{
    return 0;
}

extern double rint(double x);
double rint(double x)
{
    return 0.0f;
}


int main(int argc, char *argv[]) {
 
    isRTG = 0;      // Default to AGA

    // find out what type of CPU we have

    if ((SysBase->AttnFlags & AFF_68060) != 0)
        cpu_type = 68060;
    else if ((SysBase->AttnFlags & AFF_68040) != 0)
        cpu_type = 68040;
    else if ((SysBase->AttnFlags & AFF_68030) != 0)
        cpu_type = 68030;
    else if ((SysBase->AttnFlags & AFF_68020) != 0)
        cpu_type = 68020;
    else if ((SysBase->AttnFlags & AFF_68010) != 0)
        cpu_type = 68010;
    else
        cpu_type = 68000;
 
    if (cpu_type < 68020)
    {
        PLOG("\nYou need a 68020 or higher to run this engine\n\n");
        return 0;
    }
	packfile_mode(0);

	dirExists(paksDir, 1);
	dirExists(savesDir, 1);
	dirExists(logsDir, 1);
	dirExists(screenShotsDir, 1);
	
    if(argc > 1) {
        
        strcpy(packfile, argv[1]);
           
        if (is_dir(argv[1]))
        {
         
            if (packfile[strlen(packfile) - 1] != '/')
                strcat(packfile , "/");
             
        }  
        
        if (argc > 2)
        {
            if (strcmp(strlwr(argv[2]), "-cgx") == 0)
            {
                PLOG("OpenBOR : RTG screenmode specified\n");
                isRTG = 1;    
            }
        }      	   
    }

	openborMain(argc, argv);
	borExit(0);
	return 0;
}
