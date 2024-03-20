#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include <SDL2/SDL.h>

void graphics_init (SDL_Renderer *renderer, SDL_Window *window);

void graphics_update (SDL_Renderer *renderer);

void graphics_draw (SDL_Renderer *renderer, bool display[64][32]);

void graphics_teardown (SDL_Renderer *renderer, SDL_Window *window);

void get_keypad_states (bool keypad[16], const Uint8 *state);

#endif