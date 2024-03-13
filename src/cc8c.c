#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

void init(unsigned char* RAMptr);
void fetch ();

// Memory
uint8_t RAM[4096] = {0}; // RAM is 8bitsx4Kbits

uint16_t I = 0;
uint8_t V[16] = {0};

uint16_t pc = 0;
uint32_t opcode = 0x0000;

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

int main()
{
    init(&RAM[0]);
    
    //Fetch
    fetch();
    
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
                // redraw display
            }
            else if (opcode == 0x00EE) {
                stackPtr--;
                pc = stack[stackPtr];
            }
            else {
                printf("Error: 0NNN instruction.");
                return -1;
            }
            break;
        case 0x1: // Jump
            pc = NNN;
            break;
        case 0x2:
            stack[stackPtr] = pc;
            pc = NNN;
            if (stackPtr == 15) {
                printf("Error: stack overflow");
                return -1;
            }
            stackPtr++;
            break;
        case 0x6:
            V[x] = NN;
            break;
        case 0x7:
            V[x] += NN;
            break;
        case 0xA:
            I = NNN;
            break;
        case 0xD:
            // TODO: display
            uint8_t xCoord = V[x] & 63;
            uint8_t yCoord = V[y] & 31;
            V[0xF] = 0;

            uint8_t currRow;
            bool currBit;

            for (int i = 0; i < N; i++) {
                currRow = RAM[I + i*8];
                for (int j = 0; j < 8; j++) {
                    currBit = currRow >> j & 0x1;
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

            

        default:
            printf("Not A");
            break;
    }
    
    return 0;
}

void fetch () {
    opcode = RAM[pc];
    pc += 2;
}

void init(unsigned char* RAMptr) {
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