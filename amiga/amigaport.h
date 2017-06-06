/*
 * OpenBOR - http://www.LavaLit.com
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in OpenBOR root for details.
 *
 * Copyright (c) 2004 - 2011 OpenBOR Team
 */

#ifndef AMIGAPORT_H
#define AMIGAPORT_H
 
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include "globals.h"

#define stricmp  strcasecmp
#define strnicmp strncasecmp
 
#define paksDir "Paks"


//#define MEMTEST 1

void initAmiga();
void borExit(int reset);
void openborMain(int argc, char **argv);

extern char packfile[128];
extern char savesDir[128];
extern char logsDir[128];
extern char screenShotsDir[128];

#endif
