#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <signal.h>
#include <time.h>

/*
Display constants
*/
#define UPSCALE_MULTIPLIER (10)

#define DISPLAY_ON_R (123)
#define DISPLAY_ON_G (61)
#define DISPLAY_ON_B (17)

#define DISPLAY_OFF_R (182)
#define DISPLAY_OFF_G (152)
#define DISPLAY_OFF_B (109)

/*
Program execution speed
*/
#define INSTRUCTIONS_PER_SECOND (700)