 
#include "timer.h"

#define GETTIME_FREQ (1000)

static unsigned lastinterval = 0;


void borTimerInit() {
    
    initTimer();
    
}
void borTimerExit() {
    exitTimer();
}


unsigned timer_gettick() {
	return getMilliseconds();
}


unsigned timer_getinterval(unsigned freq) {
	unsigned tickspassed, ebx, blocksize, now;
	now = timer_gettick();
	ebx = now - lastinterval;
	blocksize = GETTIME_FREQ / freq;
	ebx += GETTIME_FREQ % freq;
	tickspassed = ebx / blocksize;
	ebx -= ebx % blocksize;
	lastinterval += ebx;
	return tickspassed;
}
