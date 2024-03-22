#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "../cc8c.h"

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
    uint32_t elapsedCycles;
} chip8_core;

void update_timers (chip8_core *chip8_core);

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
        0x60, 0x90, 0x90, 0x90, 0x60, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xE0, 0x10, 0x70, 0x80, 0xF0, // 2
        0xE0, 0x10, 0xF0, 0x10, 0xE0, // 3
        0x90, 0x90, 0x70, 0x10, 0x10, // 4
        0xF0, 0x80, 0xE0, 0x10, 0xE0, // 5
        0x70, 0x80, 0xE0, 0x90, 0x60, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0x60, 0x90, 0x60, 0x90, 0x60, // 8
        0x60, 0x90, 0x70, 0x10, 0x60, // 9
        0x60, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0x60, 0x90, 0x80, 0x90, 0x60, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xD0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xD0, 0x80, 0x80  // F
    };
    memcpy(chip8_core->RAM + 0x50, &fontSet[0], sizeof(fontSet));
    printf("Status: Font set loaded into RAM.\n");

    return;
}

void fetch (chip8_core *chip8_core) {

    chip8_core->opcode = (chip8_core->RAM[chip8_core->pc] << 8) | chip8_core->RAM[chip8_core->pc+1];
    
    printf("[0x%.2X] - 0x%.4X - ", chip8_core->pc, chip8_core->opcode);
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
                printf("Clear screen");
                memset(&chip8_core->display, 0, 64 * 32);
                chip8_core->displayUpdated = true;
            }
            else if (chip8_core->opcode == 0x00EE) {
                printf("Returning from subroutine call");
                chip8_core->stackPtr--;
                chip8_core->pc = chip8_core->stack[chip8_core->stackPtr];   
            }
            else {
                printf("ERROR: 0NNN instruction.\n");
                exit(0);
            }
            break;

        case 0x1:
            printf("Jump to RAM[0x%.3X]", NNN);
            chip8_core->pc = NNN;
            chip8_core->increment = false;
            break;

        case 0x2:
            printf("Subroutine call to RAM[0x%.3X]", NNN);
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
            printf("Skip if V[x] == NN (0x%.2X == 0x%.2X)", chip8_core->V[x], NN);
            if (chip8_core->V[x] == NN) {
                chip8_core->skip = true;
            }
            break;

        case 0x4:
            printf("Skip if V[x] != NN (0x%.2X != 0x%.2X)", chip8_core->V[x], NN);
            if (chip8_core->V[x] != NN) {
                chip8_core->skip = true;
            }
            break;

        case 0x5:
            printf("Skip if V[x] == V[y] (0x%.2X == 0x%.2X)", chip8_core->V[x], chip8_core->V[y]);
            if (chip8_core->V[x] == chip8_core->V[y]) {
                chip8_core->skip = true;
            }
            break;

        case 0x6:
            printf("Set V[x] to NN (V[0x%.1X] = 0x%.2X)", x, NN);
            chip8_core->V[x] = NN;
            break;

        case 0x7:
            printf("V[x] += NN (0x%.2X += 0x%.2X)", chip8_core->V[x], NN);
            chip8_core->V[x] += NN;
            break;

        case 0x8:
            switch (N) {
                case 0x0:
                    printf("Set V[x] to V[y] (V[%.2X] = 0x%.2X)", x, chip8_core->V[y]);
                    chip8_core->V[x] = chip8_core->V[y];
                    chip8_core->V[0xF] = 0;
                    break;
                case 0x1:
                    printf("Or V[x] with V[y] (V[%.2X] = %.2X | 0x%.2X)", x, chip8_core->V[x],chip8_core->V[y]);
                    chip8_core->V[x] |= chip8_core->V[y];
                    chip8_core->V[0xF] = 0;
                    break;
                case 0x2:
                    printf("And V[x] with V[y] (V[%.2X] = %.2X & 0x%.2X)", x, chip8_core->V[x],chip8_core->V[y]);
                    chip8_core->V[x] &= chip8_core->V[y];
                    chip8_core->V[0xF] = 0;
                    break;
                case 0x3:
                    printf("Xor V[x] with V[y] (V[%.2X] = %.2X ^ 0x%.2X)", x, chip8_core->V[x],chip8_core->V[y]);
                    chip8_core->V[x] ^= chip8_core->V[y];
                    chip8_core->V[0xF] = 0;
                    break;
                case 0x4:
                    printf("Add V[x] with V[y] (V[%.2X] = %.2X + 0x%.2X)", x, chip8_core->V[x],chip8_core->V[y]);
                    uint32_t sum = chip8_core->V[x] + chip8_core->V[y];
                    chip8_core->V[x] += chip8_core->V[y];
                    if (sum > 0xFF) {
                        chip8_core->V[0xF] = 1;
                    }
                    else {
                        chip8_core->V[0xF] = 0;
                    }
                    break;
                case 0x5:
                    printf("Subtract V[x] with V[y] (V[%.2X] = %.2X - 0x%.2X)", x, chip8_core->V[x], chip8_core->V[y]);
                    bool borrowXY = 0;
                    if (chip8_core->V[x] >= chip8_core->V[y]) {
                        borrowXY = 1;
                    }
                    else {
                        borrowXY = 0;
                    }
                    chip8_core->V[x] -= chip8_core->V[y];
                    chip8_core->V[0xF] = borrowXY;
                    break;
                case 0x6:
                    printf("Set V[x] to V[y] and right shift by 1 (V[%.2X] = %.2X >> 1)", x,chip8_core->V[y]);
                    bool rightShiftedOutBit = chip8_core->V[y] & 0x1;
                    chip8_core->V[x] = chip8_core->V[y] >> 1;
                    chip8_core->V[0xF] = rightShiftedOutBit;
                    break;
                case 0x7:
                    printf("Subtract V[y] with V[x] (V[%.2X] = %.2X - 0x%.2X)", x, chip8_core->V[y],chip8_core->V[x]);
                    bool borrowYX = 0;
                    if (chip8_core->V[y] >= chip8_core->V[x]) {
                        borrowYX = 1;
                    }
                    else {
                        borrowYX = 0;
                    }
                    chip8_core->V[x] = chip8_core->V[y] - chip8_core->V[x];
                    chip8_core->V[0xF] = borrowYX;
                    break;
                case 0xE:
                    printf("Set V[x] to V[y] and left shift by 1 (V[%.2X] = %.2X << 1)", x,chip8_core->V[y]);
                    bool leftShiftedOutBit = chip8_core->V[y] >> 7 & 0x1;
                    chip8_core->V[x] = chip8_core->V[y] << 1;
                    chip8_core->V[0xF] = leftShiftedOutBit;
                    break;
                default:
                    printf("ERROR: Invalid opcode %d\n", chip8_core->opcode);
                    exit(0);
            }
            break;

        case 0x9:
            printf("Skip if V[x] != V[y] (%.2X != %.2X)", chip8_core->V[x], chip8_core->V[y]);
            if (chip8_core->V[x] != chip8_core->V[y]) {
                chip8_core->skip = true;
            }
            break;

        case 0xA:
            printf("Set I to 0x%.3X", NNN);
            chip8_core->I = NNN;
            break;

        case 0xB:
            printf("Set PC to NNN + V[0] (0x%.3X + 0x%.2X)", NNN, chip8_core->V[0]);
            chip8_core->pc = NNN + chip8_core->V[0];
            chip8_core->increment = false;
            break;

        case 0xC:
            printf("Set V[x] to random number");
            chip8_core->V[x] = (rand() % 256) & NN;
            break;

        case 0xD:
            printf("Draw sprite");
            uint8_t xCoord = chip8_core->V[x] % 64;
            uint8_t yCoord = chip8_core->V[y] % 32;
            chip8_core->V[0xF] = 0;

            uint8_t currRow;
            bool currBit;

            for (int i = 0; i < N; i++) {
                currRow = chip8_core->RAM[chip8_core->I + i];
                xCoord = chip8_core->V[x] % 64;
                for (int j = 0; j < 8; j++) {
                    currBit = (currRow >> 7-j) & 0x1;
                    // TODO: this is xor
                    if (currBit && chip8_core->display[xCoord][yCoord]) {
                        chip8_core->display[xCoord][yCoord] = 0;
                        chip8_core->V[0xF] = 1;
                    }
                    else if (currBit && !chip8_core->display[xCoord][yCoord]) {
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
                printf("Skip if key %.1X is pressed", chip8_core->V[x]);
                if (keypad[chip8_core->V[x]]) {
                    chip8_core->skip = true;
                }
            }
            else if (NN == 0xA1) {
                printf("Skip if key %.1X is not pressed", chip8_core->V[x]);
                if (!keypad[chip8_core->V[x]]) {
                    chip8_core->skip = true;
                }
            }
            break;

        case 0xF:
            if (NN == 0x07) {
                printf("Set V[x] to delayTimer (V[%.2X] = %.2X)", x, chip8_core->delayTimer);
                chip8_core->V[x] = chip8_core->delayTimer;
            }
            else if (NN == 0x15) {
                printf("Set delayTimer to V[x] (delayTimer = V[%.2X] = %.2X)", x, chip8_core->V[x]);
                chip8_core->delayTimer = chip8_core->V[x];
            }
            else if (NN == 0x18) {
                printf("Set V[x] to soundTimer (V[%.2X] = %.2X)", x, chip8_core->soundTimer);
                chip8_core->soundTimer = chip8_core->V[x];
            }
            else if (NN == 0x1E) {
                printf("Add I with V[x] (I = %.2X + %.2X)", chip8_core->I, chip8_core->V[x]);
                chip8_core->I += chip8_core->V[x];
            }
            else if (NN == 0x0A) {
                printf("Wait for keypress");
                bool anyKeyPressed = false;
                for (int i = 0; i <= 0xF; i++) {
                    anyKeyPressed |= keypad[i];
                }
                
                if (!anyKeyPressed) {
                    chip8_core->pc -= 2;
                }
            }
            else if (NN == 0x29) {
                uint16_t address = (chip8_core->V[x] * 5) + 0x50;
                printf("Set I to address of hex character in V[x] (I = %.4x)", address);
                chip8_core->I = address;
            }
            else if (NN == 0x33) {
                int digit2 = (chip8_core->V[x] / 100) % 100;
                int digit1 = (chip8_core->V[x] / 10) % 10;
                int digit0 = chip8_core->V[x] - digit2*100 - digit1*10;
                printf("Store 3-digit BCD in V[x] to RAM starting at I");
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
    update_timers(chip8_core);
    printf("\n");
    return;
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

void update_timers (chip8_core *chip8_core) {

    if (chip8_core->elapsedCycles > (INSTRUCTIONS_PER_SECOND/60.0)) {
        if (chip8_core->delayTimer != 0) {
            chip8_core->delayTimer--;
        }
        if (chip8_core->soundTimer != 0){ 
            chip8_core->soundTimer--;
        }
        chip8_core->elapsedCycles = 0;
    }
    else {
        chip8_core->elapsedCycles++;
    }

}