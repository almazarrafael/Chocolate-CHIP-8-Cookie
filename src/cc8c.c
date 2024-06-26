#include"cc8c.h"
#include "chip8_core/chip8_core.h"
#include "graphics/graphics.h"

/*
Handles the 'ctrl+c' interrupt for exiting the program.
*/
void int_handler (int sig);

/*
Parses through the arguments provided and handles them accordingly.
*/
void arg_handler (int argc, char *argv[]);

/*
Loads the CHIP8 core with the splash screen ROM and halts until any key is pressed.
*/
void splash_screen (SDL_Renderer *renderer, const Uint8 *state);

/*
Prints the contents of instructions.txt.
*/
void print_instructions (void);

/*
Emulator state variables.
*/
bool keepRunning = true;
bool keypad[16] = {0};
bool prevKeyPState = false;
bool singleStepping = false;
bool startKeyPressed = false;

int main (int argc, char *argv[]) {

    print_instructions();

    arg_handler(argc, argv);

    signal(SIGINT, int_handler);

    // SDL init
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_CreateWindowAndRenderer(64 * UPSCALE_MULTIPLIER, 32 * UPSCALE_MULTIPLIER, 0, &window, &renderer);
    graphics_init(renderer, window);
    const Uint8 *state = SDL_GetKeyboardState(NULL);  

    if (!singleStepping) splash_screen(renderer, state);
    
    // Chip8 core init
    chip8_core *chip8_core = malloc(sizeof(struct chip8_core));
    init(chip8_core, singleStepping);
    load_rom(chip8_core, argv[1]);

    if (singleStepping) printf("STATUS: Single stepping debug mode. Press 'p' to step through.\n");

    while (keepRunning) {

        // Single stepping debug
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

        // Fetch, decode, execute
        fetch(chip8_core);
        get_keypad_states(keypad, state);
        decode_and_execute(chip8_core, keypad);

        // Graphics
        if (get_displayUpdated(chip8_core)) {
            graphics_draw(renderer, chip8_core->display);
            reset_displayUpdated(chip8_core);
        }
        graphics_update(renderer);

        // Reset button
        if (state[SDL_SCANCODE_O]) {
            printf("STATUS: Resetting..");
            reset(chip8_core, argv[1]);
        }
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

void splash_screen (SDL_Renderer *renderer, const Uint8 *state) {
    chip8_core *chip8_core = malloc(sizeof(struct chip8_core));
    init(chip8_core, singleStepping);
    load_rom(chip8_core, "../roms/splash_screen/splash_screen.ch8");

    while (!startKeyPressed) {
        // Inputs
        SDL_PumpEvents();
        get_keypad_states(keypad, state);
        for (int i = 0; i < 0xF; i++) {
            startKeyPressed |= keypad[i];
        }

        // Fetch, decode, execute
        fetch(chip8_core);
        decode_and_execute(chip8_core, keypad);
        
        // Graphics
        if (get_displayUpdated(chip8_core)) {
            graphics_draw(renderer, chip8_core->display);
            reset_displayUpdated(chip8_core);
        }
        graphics_update(renderer);
    }
    free(chip8_core);
    return;
}

void print_instructions (void) {
    FILE *file;
    uint8_t *buffer;
    unsigned long fileLen;

    file = fopen("./instructions.txt", "rb");

    if (!file) {
        printf("ERROR: can't read file. The instructions file must have been deleted. Try resetting the repo.\n");
        exit(0);
    }

    fseek(file, 0, SEEK_END);
    fileLen = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (uint8_t *)malloc(fileLen+1);
    fread(buffer, fileLen, 1, file);
    fclose(file);

    for (int c = 0; c < fileLen; c++) {
        printf("%c", (char)buffer[c]);
    }

    free(buffer);
    return;
}