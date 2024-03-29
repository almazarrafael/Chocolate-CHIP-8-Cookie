#ifndef CHIP8_CORE_H_
#define CHIP8_CORE_H_

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
    uint32_t elapsedCycles;
    bool debug;
} chip8_core;

/*
Initializes the chip8 core to default values.
*/
void init(chip8_core *chip8_core, bool debug);

/*
Fetches opcode using PC. Also increments PC according to previous instruction.
*/
void fetch (chip8_core *chip8_core);

/*
Decode breaks down the opcode into different parts. Execute runs the corresponding instructions depending on the opcode.
*/
void decode_and_execute (chip8_core *chip8_core, bool keypad[16]);

/*
Loads rom into RAM using the ROM provided in romPath.
*/
void load_rom (chip8_core *chip8_core, char* romPath);

/*
Getter for displayUpdated in the chip8 core struct. For use with a display drawing function.
*/
bool get_displayUpdated (chip8_core *chip8_core);

/*
Resets displayUpdated after drawing.
*/
void reset_displayUpdated (chip8_core *chip8_core);

/*
Resets the chip8 core back to default values and reloads the ROM.
*/
void reset (chip8_core *chip8_core, char* romPath);

#endif