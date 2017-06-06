
#include "video.h"
#include "globals.h"
#include "control.h"
#include "stristr.h"
#include "sblaster.h"
#include "joysticks.h"
#include "openbor.h"


/*
Reset All data back to Zero and
destroy all SDL Joystick data.
*/
void control_exit() {
 
}


/*
Create default values for joysticks if enabled.
Then scan for joysticks and update their data.
*/
void control_init(int joy_enable) {
 
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
	return 0;
}


void control_setkey(s_playercontrols * pcontrols, unsigned int flag, int key) {
 
}


// Scan input for newly-pressed keys.
// Return value:
// 0  = no key was pressed
// >0 = key code for pressed key
// <0 = error

int control_scankey() {
 
	return 0;
}


void control_update(s_playercontrols ** playercontrols, int numplayers) {
 
}


char *control_getkeyname(unsigned int keycode) {
 
    return "";
}


int keyboard_getlastkey() {
 
	return 0;
}
