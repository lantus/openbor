/*
 * OpenBOR - http://www.LavaLit.com
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in OpenBOR root for details.
 *
 * Copyright (c) 2004 - 2011 OpenBOR Team
 */

#ifndef	CONTROL_H
#define	CONTROL_H
 
#include "joysticks.h" 

/* Port types */
#define JP_TYPE_NOTAVAIL  (00<<28)	  /* port data unavailable    */
#define JP_TYPE_GAMECTLR  (01<<28)	  /* port has game controller */
#define JP_TYPE_MOUSE	  (02<<28)	  /* port has mouse	      */
#define JP_TYPE_JOYSTK	  (03<<28)	  /* port has joystick	      */
#define JP_TYPE_UNKNOWN   (04<<28)	  /* port has unknown device  */
#define JP_TYPE_MASK	  (15<<28)	  /* controller type	      */

/* Button types, valid for all types except JP_TYPE_NOTAVAIL */
#define JPB_BUTTON_BLUE    23	  /* Blue - Stop; Right Mouse		     */
#define JPB_BUTTON_RED	   22	  /* Red - Select; Left Mouse; Joystick Fire */
#define JPB_BUTTON_YELLOW  21	  /* Yellow - Repeat			     */
#define JPB_BUTTON_GREEN   20	  /* Green - Shuffle			     */
#define JPB_BUTTON_FORWARD 19	  /* Charcoal - Forward		     */
#define JPB_BUTTON_REVERSE 18	  /* Charcoal - Reverse		     */
#define JPB_BUTTON_PLAY    17	  /* Grey - Play/Pause; Middle Mouse	     */
#define JPF_BUTTON_BLUE    (1<<JPB_BUTTON_BLUE)
#define JPF_BUTTON_RED	   (1<<JPB_BUTTON_RED)
#define JPF_BUTTON_YELLOW  (1<<JPB_BUTTON_YELLOW)
#define JPF_BUTTON_GREEN   (1<<JPB_BUTTON_GREEN)
#define JPF_BUTTON_FORWARD (1<<JPB_BUTTON_FORWARD)
#define JPF_BUTTON_REVERSE (1<<JPB_BUTTON_REVERSE)
#define JPF_BUTTON_PLAY    (1<<JPB_BUTTON_PLAY)
#define JP_BUTTON_MASK	   (JPF_BUTTON_BLUE|JPF_BUTTON_RED|JPF_BUTTON_YELLOW|JPF_BUTTON_GREEN|JPF_BUTTON_FORWARD|JPF_BUTTON_REVERSE|JPF_BUTTON_PLAY)

/* Direction types, valid for JP_TYPE_GAMECTLR and JP_TYPE_JOYSTK */
#define JPB_JOY_UP	  3
#define JPB_JOY_DOWN	  2
#define JPB_JOY_LEFT	  1
#define JPB_JOY_RIGHT	  0
#define JPF_JOY_UP	  	  (1<<JPB_JOY_UP)
#define JPF_JOY_DOWN	  (1<<JPB_JOY_DOWN)
#define JPF_JOY_LEFT	  (1<<JPB_JOY_LEFT)
#define JPF_JOY_RIGHT	  (1<<JPB_JOY_RIGHT)
#define JP_DIRECTION_MASK (JPF_JOY_UP|JPF_JOY_DOWN|JPF_JOY_LEFT|JPF_JOY_RIGHT)

/* Mouse position reports, valid for JP_TYPE_MOUSE */
#define JP_MHORZ_MASK	(255<<0)	/* horzizontal position */
#define JP_MVERT_MASK	(255<<8)	/* vertical position	*/
#define JP_MOUSE_MASK	(JP_MHORZ_MASK|JP_MVERT_MASK)

/* Obsolete ReadJoyPort() definitions, here for source code compatibility only.
 * Please do NOT use in new code.
 */
#define JPB_BTN1  JPB_BUTTON_BLUE
#define JPF_BTN1  JPF_BUTTON_BLUE
#define JPB_BTN2  JPB_BUTTON_RED
#define JPF_BTN2  JPF_BUTTON_RED
#define JPB_BTN3  JPB_BUTTON_YELLOW
#define JPF_BTN3  JPF_BUTTON_YELLOW
#define JPB_BTN4  JPB_BUTTON_GREEN
#define JPF_BTN4  JPF_BUTTON_GREEN
#define JPB_BTN5  JPB_BUTTON_FORWARD
#define JPF_BTN5  JPF_BUTTON_FORWARD
#define JPB_BTN6  JPB_BUTTON_REVERSE
#define JPF_BTN6  JPF_BUTTON_REVERSE
#define JPB_BTN7  JPB_BUTTON_PLAY
#define JPF_BTN7  JPF_BUTTON_PLAY
#define JPB_UP	  JPB_JOY_UP
#define JPF_UP	  JPF_JOY_UP
#define JPB_DOWN  JPB_JOY_DOWN
#define JPF_DOWN  JPF_JOY_DOWN
#define JPB_LEFT  JPB_JOY_LEFT
#define JPF_LEFT  JPF_JOY_LEFT
#define JPB_RIGHT JPB_JOY_RIGHT
#define JPF_RIGHT JPF_JOY_RIGHT


#define	CONTROL_DEFAULT1_UP	 	 JPF_JOY_UP
#define	CONTROL_DEFAULT1_DOWN	 JPF_JOY_DOWN
#define	CONTROL_DEFAULT1_LEFT	 JPF_JOY_LEFT
#define	CONTROL_DEFAULT1_RIGHT	 JPF_JOY_RIGHT
#define	CONTROL_DEFAULT1_SPECIAL 0x400
#define CONTROL_DEFAULT1_ATTACK  JPF_BUTTON_RED
#define CONTROL_DEFAULT1_JUMP    JPF_BUTTON_BLUE
#define CONTROL_DEFAULT1_START   0x4000
#define CONTROL_DEFAULT1_SCREENSHOT (0x800)

#define CONTROL_DEFAULT2_UP      (0x800)
#define CONTROL_DEFAULT2_DOWN    (0x800)
#define CONTROL_DEFAULT2_LEFT    (0x800)
#define CONTROL_DEFAULT2_RIGHT   (0x800)
#define CONTROL_DEFAULT2_SPECIAL (0x800)
#define CONTROL_DEFAULT2_ATTACK  (0x800)
#define CONTROL_DEFAULT2_JUMP    (0x800)
#define CONTROL_DEFAULT2_START   (0x800)
#define CONTROL_DEFAULT2_SCREENSHOT (0x800)



typedef struct {
	int settings[JOY_MAX_INPUTS];
	u32 keyflags, newkeyflags;
	int kb_break;
} s_playercontrols;

void control_exit();
void control_init(int joy_enable);
int control_usejoy(int enable);
int control_getjoyenabled();

void control_setkey(s_playercontrols * pcontrols, unsigned int flag, int key);
int control_scankey();

char *control_getkeyname(unsigned int keycode);
void control_update(s_playercontrols ** playercontrols, int numplayers);
void control_rumble(int port, int msec);
int keyboard_getlastkey();

#endif
