#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include <SDL2/SDL.h>

/*
Initializes the drawing window and clears it.
*/
void graphics_init (SDL_Renderer *renderer, SDL_Window *window);

/*
Updates input array and delays the program.
*/
void graphics_update (SDL_Renderer *renderer);

/*
Updates and renders the drawing based on the current state of the display array.
*/
void graphics_draw (SDL_Renderer *renderer, bool display[64][32]);

/*
Frees graphics for clean up on program exit.
*/
void graphics_teardown (SDL_Renderer *renderer, SDL_Window *window);

/*
Populates the keypad array with the states of the Chip8 keypad.
*/
void get_keypad_states (bool keypad[16], const Uint8 *state);

#endif