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
} chip8_core;

void init(chip8_core *chip8_core);

void fetch (chip8_core *chip8_core);

void decode_and_execute (chip8_core *chip8_core, bool keypad[16]);

void load_rom (chip8_core *chip8_core, char* romPath);

bool get_displayUpdated (chip8_core *chip8_core);

void reset_displayUpdated (chip8_core *chip8_core);

void update_timers (chip8_core *chip8_core);

#endif