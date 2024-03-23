#include"cc8c.h"
#include "chip8_core/chip8_core.h"
#include "graphics/graphics.h"

void int_handler (int sig);
void arg_handler (int argc, char *argv[]);

bool keepRunning = true;
bool keypad[16] = {0};
bool prevKeyPState = false;
bool singleStepping = false;

int main (int argc, char *argv[]) {

    arg_handler(argc, argv);

    signal(SIGINT, int_handler);

    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_CreateWindowAndRenderer(64 * UPSCALE_MULTIPLIER, 32 * UPSCALE_MULTIPLIER, 0, &window, &renderer);
    
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    graphics_init(renderer, window);

    chip8_core *chip8_core = malloc(sizeof(struct chip8_core));
    init(chip8_core);
    load_rom(chip8_core, argv[1]);

    if (singleStepping) printf("STATUS: Single stepping debug mode. Press 'P' to step through.\n");

    while (keepRunning) {

        if (singleStepping) {
            while (true) {
                if (prevKeyPState && !state[SDL_SCANCODE_P]) {
                    prevKeyPState = false;
                    break;
                }
                if (!keepRunning) break;
                prevKeyPState = state[SDL_SCANCODE_P];
                SDL_PumpEvents();
            }
        }

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

void arg_handler (int argc, char *argv[]) {
    if (argc == 1) {
        printf("ERROR: Please provide a path to a CHIP-8 ROM file.\n");
        printf("Usage: ./cc8c \"ROM path\"\n");
        exit(0);
    }

    if (argc > 3) {
        printf("ERROR: Too many arguments provided.\n");
        exit(0);
    }

    if (argc == 3) {
        if (!strcmp(argv[2], "-d")) {
            singleStepping = true;
        }
        else {
            printf("ERROR: Unknown flag \'%s\'. Use -d for single stepping debug mode.\n", argv[2]);
            exit(0);
        }
    }

}