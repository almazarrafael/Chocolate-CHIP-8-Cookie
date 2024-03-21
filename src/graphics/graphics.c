#include "../cc8c.h"

void graphics_init (SDL_Renderer *renderer, SDL_Window *window) {
    SDL_Init(SDL_INIT_VIDEO);
    
    SDL_SetRenderDrawColor(renderer, DISPLAY_OFF_R, DISPLAY_OFF_G, DISPLAY_OFF_B, 255);
    SDL_RenderClear(renderer);
    
    SDL_SetRenderDrawColor(renderer, DISPLAY_ON_R, DISPLAY_ON_G, DISPLAY_ON_B, 255);

    return;
}

void graphics_update (SDL_Renderer *renderer) {
    SDL_PumpEvents();
    SDL_Delay(1000/(INSTRUCTIONS_PER_SECOND * 1.0));
    return;
}

void graphics_draw (SDL_Renderer *renderer, bool display[64][32]) {

    for (int i = 0; i < 64 * UPSCALE_MULTIPLIER; i++) {
        for (int j = 0; j < 32 * UPSCALE_MULTIPLIER; j++) {
            if (display[i/UPSCALE_MULTIPLIER][j/UPSCALE_MULTIPLIER]) {
                SDL_RenderDrawPoint(renderer, i, j);
            }
            else {
                SDL_SetRenderDrawColor(renderer, DISPLAY_OFF_R, DISPLAY_OFF_G, DISPLAY_OFF_B, 255);
                SDL_RenderDrawPoint(renderer, i, j);
                SDL_SetRenderDrawColor(renderer, DISPLAY_ON_R, DISPLAY_ON_G, DISPLAY_ON_B, 255);
            }
        }
    }

    SDL_RenderPresent(renderer);
    return;
}

void graphics_teardown (SDL_Renderer *renderer, SDL_Window *window) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return;
}

void get_keypad_states (bool keypad[16], const Uint8 *state) {
    const uint32_t keys[] = {
        SDL_SCANCODE_X, // 0
        SDL_SCANCODE_1, // 1
        SDL_SCANCODE_2, // 2
        SDL_SCANCODE_3, // 3
        SDL_SCANCODE_Q, // 4
        SDL_SCANCODE_W, // 5
        SDL_SCANCODE_E, // 6
        SDL_SCANCODE_A, // 7
        SDL_SCANCODE_S, // 8
        SDL_SCANCODE_D, // 9
        SDL_SCANCODE_Z, // A
        SDL_SCANCODE_C, // B
        SDL_SCANCODE_4, // C
        SDL_SCANCODE_R, // D
        SDL_SCANCODE_F, // E
        SDL_SCANCODE_V  // F
    };
    
    for (int i = 0; i < 16; i++) {
        keypad[i] = state[keys[i]];
    }
    return;
}