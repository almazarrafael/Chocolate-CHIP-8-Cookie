#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct chip8_core {
    uint8_t RAM[4096];
    uint16_t I;
    uint8_t V[16];
    uint16_t pc;
    uint16_t opcode;
    uint16_t stack[16];
    uint8_t stackPtr;
    uint8_t delayTimer;
    uint8_t soundTimer;
    bool display[64][32];
    bool increment;
    bool skip;
    bool displayUpdated;
} chip8_core;

void init(chip8_core *chip8_core) {

    for (int i = 0; i < 4096; i++) {
        chip8_core->RAM[i] = 0;
    }

    chip8_core->I = 0;

    for (int i = 0; i < 16; i++) {
        chip8_core->V[i] = 0;
    }
    
    chip8_core->pc = 0x200;
    chip8_core->opcode = 0x0000;

    for (int i = 0; i < 16; i++) {
        chip8_core->stack[i] = 0;
    }

    chip8_core->stackPtr = 0;
    chip8_core->delayTimer = 0;
    chip8_core->soundTimer = 0;

    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 32; j++) {
            chip8_core->display[i][j] = false;
        }
    }

    chip8_core->increment = true;
    chip8_core->skip = false;
    chip8_core->displayUpdated = false;

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
    memcpy(chip8_core->RAM + 0x50, &fontSet[0], sizeof(fontSet));
    printf("Status: Font set loaded into RAM.\n");

    return;
}

void fetch (chip8_core *chip8_core) {
    chip8_core->opcode = (chip8_core->RAM[chip8_core->pc] << 8) | chip8_core->RAM[chip8_core->pc+1];
    
    if (chip8_core->skip) {
        chip8_core->pc += 2;
        chip8_core->skip = false;
    }

    if (chip8_core->increment) {
        chip8_core->pc += 2;
    }
    else {
        chip8_core->increment = true;
    } 
    return;
}

void decode_and_execute (chip8_core *chip8_core, bool keypad[16]) {
    
    // Decode
    const uint8_t MSB  = (chip8_core->opcode & 0xF000) >> 12;
    const uint8_t x    = (chip8_core->opcode & 0x0F00) >> 8;
    const uint8_t y    = (chip8_core->opcode & 0x00F0) >> 4;
    const uint8_t N    = (chip8_core->opcode & 0x000F);
    const uint8_t NN   = (chip8_core->opcode & 0x00FF);
    const uint16_t NNN = (chip8_core->opcode & 0x0FFF);

    // Execute
    switch (MSB) {

        case 0x0:
            if (chip8_core->opcode == 0x00E0) {
                memset(&chip8_core->display, 0, 64 * 32);
                chip8_core->displayUpdated = true;
            }
            else if (chip8_core->opcode == 0x00EE) {
                chip8_core->stackPtr--;
                chip8_core->pc = chip8_core->stack[chip8_core->stackPtr];   
            }
            else {
                printf("ERROR: 0NNN instruction.\n");
                exit(0);
            }
            break;

        case 0x1: // Jump
            chip8_core->pc = NNN;
            chip8_core->increment = false;
            break;

        case 0x2:
            chip8_core->stack[chip8_core->stackPtr] = chip8_core->pc;
            chip8_core->pc = NNN;
            if (chip8_core->stackPtr == 15) {
                printf("ERROR: Stack overflow.\n");
                exit(0);
            }
            chip8_core->increment = false;
            chip8_core->stackPtr++;
            break;

        case 0x3:
            if (chip8_core->V[x] == NN) {
                chip8_core->skip = true;
            }
            break;

        case 0x4:
            if (chip8_core->V[x] != NN) {
                chip8_core->skip = true;
            }
            break;

        case 0x5:
            if (chip8_core->V[x] == chip8_core->V[y]) {
                chip8_core->skip = true;
            }
            break;

        case 0x6:
            chip8_core->V[x] = NN;
            break;

        case 0x7:
            chip8_core->V[x] += NN;
            break;

        case 0x8:
            switch (N) {
                case 0x0:
                    chip8_core->V[x] = chip8_core->V[y];
                    chip8_core->V[0xF] = 0;
                    break;
                case 0x1:
                    chip8_core->V[x] |= chip8_core->V[y];
                    chip8_core->V[0xF] = 0;
                    break;
                case 0x2:
                    chip8_core->V[x] &= chip8_core->V[y];
                    chip8_core->V[0xF] = 0;
                    break;
                case 0x3:
                    chip8_core->V[x] ^= chip8_core->V[y];
                    chip8_core->V[0xF] = 0;
                    break;
                case 0x4:
                    chip8_core->V[x] += chip8_core->V[y];
                    chip8_core->V[0xF] = 0;
                    break;
                case 0x5:
                    chip8_core->V[x] -= chip8_core->V[y];
                    chip8_core->V[0xF] = 0;
                    break;
                case 0x6:
                    chip8_core->V[0xF] = chip8_core->V[y] & 0x1;
                    chip8_core->V[x] = chip8_core->V[y] >> 1;
                    break;
                case 0x7:
                    chip8_core->V[x] = chip8_core->V[y] - chip8_core->V[x];
                    chip8_core->V[0xF] = 0;
                    break;
                case 0xE:
                    chip8_core->V[0xF] = chip8_core->V[y] >> 7 & 0x1;
                    chip8_core->V[x] = chip8_core->V[y] << 1;
                    break;
                default:
                    printf("ERROR: Invalid opcode %d\n", chip8_core->opcode);
                    exit(0);
            }
            break;

        case 0x9:
            if (chip8_core->V[x] != chip8_core->V[y]) {
                chip8_core->skip = true;
            }
            break;

        case 0xA:
            chip8_core->I = NNN;
            break;

        case 0xB:
            chip8_core->pc = NNN + chip8_core->V[0];
            chip8_core->increment = false;
            break;

        case 0xC:
            chip8_core->V[x] = (rand() % 256) & NN;
            break;

        case 0xD:
            uint8_t xCoord = chip8_core->V[x] & 63;
            uint8_t yCoord = chip8_core->V[y] & 31;
            chip8_core->V[0xF] = 0;

            uint8_t currRow;
            bool currBit;

            for (int i = 0; i < N; i++) {
                currRow = chip8_core->RAM[chip8_core->I + i];
                xCoord = chip8_core->V[x] & 63;
                for (int j = 0; j < 8; j++) {
                    currBit = (currRow >> 7-j) & 0x1;
                    // TODO: this is xor
                    if (currBit && chip8_core->display[xCoord][yCoord]) {
                        chip8_core->display[xCoord][yCoord] = 0;
                        chip8_core->V[0xF] = 1;
                    }

                    if (currBit && !chip8_core->display[xCoord][yCoord]) {
                        chip8_core->display[xCoord][yCoord] = 1;
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
            chip8_core->displayUpdated = true;
            break;

        case 0xE:
            if (NN == 0x9E) {
                if (keypad[chip8_core->V[x] % 0xF]) {
                    chip8_core->skip = true;
                }
            }
            else if (NN == 0xA1) {
                if (!keypad[chip8_core->V[x] % 0xF]) {
                    chip8_core->skip = true;
                }
            }
            break;

        case 0xF:
            if (NN == 0x07) {
                chip8_core->V[x] = chip8_core->delayTimer;
            }
            else if (NN == 0x15) {
                chip8_core->delayTimer = chip8_core->V[x];
            }
            else if (NN == 0x18) {
                chip8_core->soundTimer = chip8_core->V[x];
            }
            else if (NN == 0x1E) {
                chip8_core->I += chip8_core->V[x];
            }
            else if (NN == 0x0A) {
                for (int i = 0; i < 0xF; i++) {
                    if (keypad[i]) {
                        chip8_core->pc -= 2;
                        break;
                    }
                }
            }
            else if (NN == 0x29) {
                chip8_core->I = (chip8_core->V[x] * 5) + 0x50;
            }
            else if (NN == 0x33) {
                int digit2 = (chip8_core->V[x] / 100) % 100;
                int digit1 = (chip8_core->V[x] / 10) % 10;
                int digit0 = chip8_core->V[x] - digit2*100 - digit1*10;
                chip8_core->RAM[chip8_core->I] = digit2;
                chip8_core->RAM[chip8_core->I+1] = digit1;
                chip8_core->RAM[chip8_core->I+2] = digit0;
            }
            else if (NN == 0x55) {
                for (int i = 0; i <= x; i++) {
                    chip8_core->RAM[chip8_core->I+i] = chip8_core->V[i];
                }
            }
            else if (NN == 0x65) {
                for (int i = 0; i <= x; i++) {
                    chip8_core->V[i] = chip8_core->RAM[chip8_core->I+i];
                }
            }
            break;

        default:
            printf("ERROR: Invalid OPCODE (%.4X).\n", chip8_core->opcode);
            exit(0);
            break;
    }
}

void load_rom (chip8_core *chip8_core, char *romPath) {
    FILE *file;
    uint8_t *buffer;
    unsigned long fileLen;

    file = fopen(romPath, "rb");

    if (!file) {
        printf("ERROR: can't read file.");
        exit(0);
    }

    fseek(file, 0, SEEK_END);
    fileLen = ftell(file);
    fseek(file, 0, SEEK_SET);

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

    memcpy(chip8_core->RAM+0x200, buffer, fileLen);
    free(buffer);
    return;
}

bool get_displayUpdated (chip8_core *chip8_core) {
    return chip8_core->displayUpdated;
}

void reset_displayUpdated (chip8_core *chip8_core) {
    chip8_core->displayUpdated = false;
    return;
}