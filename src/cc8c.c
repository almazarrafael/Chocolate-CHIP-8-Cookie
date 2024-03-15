#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <signal.h>
#include <time.h>

#define UPSCALE_MULTIPLIER (15)
#define DISPLAY_ON_R (123)
#define DISPLAY_ON_G (61)
#define DISPLAY_ON_B (17)
#define DISPLAY_OFF_R (182)
#define DISPLAY_OFF_G (152)
#define DISPLAY_OFF_B (109)
#define INSTRUCTIONS_PER_SECOND (700)

void init (uint8_t *RAMptr);
void fetch (void);
void draw (SDL_Renderer *renderer, bool display[64][32]);
void load_rom (void);
void int_handler (int sig);

bool keepRunning = true;
bool increment = true;

// Memory
uint8_t RAM[4096] = {0}; // RAM is 8bitsx4Kbits

uint16_t I = 0;
uint8_t V[16] = {0};

uint16_t pc = 0x200;
uint16_t opcode = 0x0000;

uint16_t stack[16] = {0};
uint8_t stackPtr = 0;

// Timers
uint8_t delayTimer = 0;
uint8_t soundTimer = 0;

// Display
bool display[64][32] = {0};

// Decode
uint8_t MSB = 0;
uint8_t x = 0;
uint8_t y = 0;
uint8_t N = 0;
uint8_t NN = 0x00;
uint16_t NNN = 0x000;

/*
KEYPAD LAYOUT
1[1] 2[2] 3[3] 4[C]
Q[4] W[5] E[6] R[D]
A[7] S[8] D[9] F[E]
Z[A] X[0] C[B] V[F]
*/

uint32_t keypad[] = {
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

int main()
{
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    signal(SIGINT, int_handler);
    init(RAM); // passing a global variable as a pointer into a function. Nice one me
    load_rom();

    SDL_Renderer *renderer;
    SDL_Window *window;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(64 * UPSCALE_MULTIPLIER, 32 * UPSCALE_MULTIPLIER, 0, &window, &renderer);
    //SDL_SetRenderDrawColor(renderer, DISPLAY_OFF_R, DISPLAY_OFF_G, DISPLAY_OFF_B, 255);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, DISPLAY_ON_R, DISPLAY_ON_G, DISPLAY_ON_B, 255);

    time_t t;
    srand((unsigned) time(&t));

    while (keepRunning) {

        // DEBUG
        // TODO: Do this periodically instead
        if (state[SDL_SCANCODE_P]) {
            SDL_RenderPresent(renderer);
        }
        
        //Fetch
        fetch();
        //printf("[0x%.3X]: 0x%.4X\n", pc, opcode);

        for (int i = 0; i < 0xF; i++) {
            printf("%d ", state[keypad[i]]);
        }
        printf("\n");

        // Decode
        MSB = (opcode & 0xF000) >> 12;
        x   = (opcode & 0x0F00) >> 8;
        y   = (opcode & 0x00F0) >> 4;
        N   = (opcode & 0x000F);
        NN  = (opcode & 0x00FF);
        NNN = (opcode & 0x0FFF);
        
        // Execute
        switch (MSB) {

            case 0x0:
                if (opcode == 0x00E0) {
                    memset(&display, 0, 64 * 32);
                    draw(renderer, display);
                }
                else if (opcode == 0x00EE) {
                    stackPtr--;
                    pc = stack[stackPtr];   
                }
                else {
                    printf("ERROR: 0NNN instruction.\n");
                    exit(0);
                }
                break;

            case 0x1: // Jump
                pc = NNN;
                increment = false;
                break;

            case 0x2:
                stack[stackPtr] = pc;
                pc = NNN;
                if (stackPtr == 15) {
                    printf("ERROR: Stack overflow.\n");
                    exit(0);
                }
                increment = false;
                stackPtr++;
                break;

            case 0x3:
                if (V[x] == NN) {
                    pc+=2;
                }
                break;

            case 0x4:
                if (V[x] != NN) {
                    pc+=2;
                }
                break;

            case 0x5:
                if (V[x] == V[y]) {
                    pc+=2;
                }
                break;

            case 0x6:
                V[x] = NN;
                break;

            case 0x7:
                V[x] += NN;
                break;

            case 0x8:
                switch (N) {
                    case 0x0:
                        V[x] = V[y];
                        V[0xF] = 0;
                        break;
                    case 0x1:
                        V[x] |= V[y];
                        V[0xF] = 0;
                        break;
                    case 0x2:
                        V[x] &= V[y];
                        V[0xF] = 0;
                        break;
                    case 0x3:
                        V[x] ^= V[y];
                        V[0xF] = 0;
                        break;
                    case 0x4:
                        V[x] += V[y];
                        V[0xF] = 0;
                        break;
                    case 0x5:
                        V[x] -= V[y];
                        V[0xF] = 0;
                        break;
                    case 0x6:
                        V[0xF] = V[y] & 0x1;
                        V[x] = V[y] >> 1;
                        break;
                    case 0x7:
                        V[x] = V[y] - V[x];
                        V[0xF] = 0;
                        break;
                    case 0xE:
                        V[0xF] = V[y] >> 7 & 0x1;
                        V[x] = V[y] << 1;
                        break;
                    default:
                        printf("ERROR: Invalid opcode %d\n", opcode);
                        exit(0);
                }
                break;

            case 0x9:
                if (V[x] != V[y]) {
                    pc+=2;
                }
                break;

            case 0xA:
                I = NNN;
                break;

            case 0xB:
                pc = NNN + V[0];
                increment = false;
                break;

            case 0xC:
                V[x] = (rand() % 256) & NN;
                break;

            case 0xD:
                uint8_t xCoord = V[x] & 63;
                uint8_t yCoord = V[y] & 31;
                V[0xF] = 0;

                uint8_t currRow;
                bool currBit;

                for (int i = 0; i < N; i++) {
                    currRow = RAM[I + i];
                    xCoord = V[x] & 63;
                    for (int j = 0; j < 8; j++) {
                        //printf("i = %d\n", j);
                        currBit = (currRow >> 7-j) & 0x1;
                        // TODO: this is xor
                        if (currBit && display[xCoord][yCoord]) {
                            display[xCoord][yCoord] = 0;
                            V[0xF] = 1;
                        }

                        if (currBit && !display[xCoord][yCoord]) {
                            display[xCoord][yCoord] = 1;
                        }
                        
                        if (xCoord == 63) {
                            break;
                        }
                        xCoord++;
                    }
                    if (yCoord == 31) {
                        break;
                    }
                    yCoord++;
                }
                draw(renderer, display);
                break;

            case 0xE:
                if (NN == 0x9E) {
                    if (state[keypad[V[x] % 0xF]]) {
                        pc += 2;
                    }
                }
                else if (NN == 0xA1) {
                    if (!state[keypad[V[x] % 0xF]]) {
                        pc += 2;
                    }
                }
                break;

            case 0xF:
                if (NN == 0x07) {
                    V[x] = delayTimer;
                }
                else if (NN == 0x15) {
                    delayTimer = V[x];
                }
                else if (NN == 0x18) {
                    soundTimer = V[x];
                }
                else if (NN == 0x1E) {
                    I += V[x];
                }
                else if (NN == 0x0A) {
                    bool anyKeyIsPressed = false;
                    for (int i = 0; i < 0xF; i++) {
                        anyKeyIsPressed |= state[keypad[i]];
                    }
                    if (!anyKeyIsPressed) {
                        pc -= 2;
                    }
                }
                else if (NN == 0x29) {
                    I = (V[x] * 5) + 0x50;
                }
                else if (NN == 0x33) {
                    int digit2 = (V[x] / 100) % 100;
                    int digit1 = (V[x] / 10) % 10;
                    int digit0 = V[x] - digit2*100 - digit1*10;
                    RAM[I] = digit2;
                    RAM[I+1] = digit1;
                    RAM[I+2] = digit0;
                }
                else if (NN == 0x55) {
                    for (int i = 0; i <= x; i++) {
                        RAM[I+i] = V[i];
                    }
                }
                else if (NN == 0x65) {
                    for (int i = 0; i <= x; i++) {
                        V[i] = RAM[I+i];
                    }
                }
                break;

            default:
                printf("ERROR: Invalid OPCODE (%.4X).\n", opcode);
                exit(0);
                break;
        }
        SDL_PumpEvents();
        SDL_Delay(1000/(INSTRUCTIONS_PER_SECOND * 1.0));
        // Refreshes screen. But somehow breaks the background color? Should do this periodically instead
        //SDL_RenderPresent(renderer);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void fetch (void) {
    opcode = (RAM[pc] << 8) | RAM[pc+1];
    if (increment) {
        pc += 2;
    }
    else {
        increment = true;
    }
    
}

void init(uint8_t *RAMptr) {
    char fontSet[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 1
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 2
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 3
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 4
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 5
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 6
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 7
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 8
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 9
        0xF0, 0x90, 0x90, 0x90, 0xF0, // A
        0xF0, 0x90, 0x90, 0x90, 0xF0, // B
        0xF0, 0x90, 0x90, 0x90, 0xF0, // C
        0xF0, 0x90, 0x90, 0x90, 0xF0, // D
        0xF0, 0x90, 0x90, 0x90, 0xF0, // E
        0xF0, 0x90, 0x90, 0x90, 0xF0 // F
    };
    memcpy(RAMptr+0x50, &fontSet[0], sizeof(fontSet));
    return;
}

void draw(SDL_Renderer *renderer, bool display[64][32]) {

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
}

void load_rom (void) {
    FILE *file;
    uint8_t *buffer;
    unsigned long fileLen;

    // TODO: Pass rom path as an arg
    //file = fopen("../roms/IBM_Logo.ch8", "rb");
    //file = fopen("../roms/3-corax+.ch8", "rb");
    //file = fopen("../roms/octojam1title.ch8", "rb");
    //file = fopen("../roms/RPS.ch8", "rb");
    file = fopen("../roms/4-flags.ch8", "rb");
    //file = fopen("../roms/6-keypad.ch8", "rb");

    if (!file) {
        printf("ERROR: can't read file.");
        exit(0);
    }

    fseek(file, 0, SEEK_END);
    fileLen = ftell(file);
    fseek(file, 0, SEEK_SET);

    printf("FILELEN:%ld\n", fileLen);

    buffer = (uint8_t *)malloc(fileLen+1);
    fread(buffer, fileLen, 1, file);
    fclose(file);

    for (int c=0;c<fileLen;c++) {
        printf("%.2X", (int)buffer[c]);

        if (c % 2 == 1)
        {
            printf(" ");
        }

        if (c % 16 == 15)
        {
            printf("\n");
        }
    }
    printf("\n");

    memcpy(RAM+0x200, buffer, fileLen);
    free(buffer);
    return;
}

void int_handler (int sig) {
    printf("Exiting program..\n");
    keepRunning = false;
}