#include <stdio.h>
#include <string.h>
#include <stdbool.h>

void init(unsigned char* RAMptr);
void fetch ();

// Memory
unsigned char RAM[4096] = {0}; // RAM is 8bitsx4Kbits

unsigned short I = 0;
unsigned char V[16] = {0};

unsigned short pc = 0;
unsigned int opcode = 0x0000;

unsigned short stack[16] = {0};
unsigned char stackPtr = 0;

// Timers
unsigned char delayTimer = 0;
unsigned char soundTimer = 0;

// Display
bool display[64][32] = {0};

// Decode
unsigned char MSB = 0;
unsigned char x = 0;
unsigned char y = 0;
unsigned char N = 0;
unsigned char NN = 0x00;
unsigned short NNN = 0x000;

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