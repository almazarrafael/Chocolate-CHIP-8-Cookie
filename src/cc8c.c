#include"cc8c.h"
#include "chip8_core/chip8_core.h"
#include "graphics/graphics.h"

void int_handler (int sig);
void draw (SDL_Renderer *renderer, bool display[64][32]);

bool keepRunning = true;
bool keypad[16] = {0};

int main (void) {

    signal(SIGINT, int_handler);

    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_CreateWindowAndRenderer(64 * UPSCALE_MULTIPLIER, 32 * UPSCALE_MULTIPLIER, 0, &window, &renderer);
    
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    graphics_init(renderer, window);

    chip8_core *chip8_core = malloc(sizeof(struct chip8_core));
    init(chip8_core);
    load_rom(chip8_core, "../roms/1-chip8-logo.ch8");

    while (keepRunning) {

        fetch(chip8_core);
        get_keypad_states(keypad, state);
        decode_and_execute(chip8_core, keypad);

        if (get_displayUpdated(chip8_core)) {
            graphics_draw(renderer, chip8_core->display);
            reset_displayUpdated(chip8_core);
        }

        graphics_update(renderer);
    }
    graphics_teardown(renderer, window);
}

void int_handler (int sig) {
    printf("Exiting program..\n");
    keepRunning = false;
}