#pragma once

#include <array>
#include <vector>
#include <stdint.h>

const uint8_t SCREEN_WIDTH = 64;
const uint8_t SCREEN_HEIGHT = 32;

const uint16_t PROG_START_ADDR = 0x200;
const uint8_t  FONT_START_ADDR = 0x050;

// 16 built-in characters, 5 bytes each
const uint8_t FONTSET_SIZE = 80;

class CPU
{
  private:
    // 16 8-bit registers
    std::array<uint8_t, 16> V;

    // 4kb RAM
    std::array<uint8_t, 4096> memory;

    // Program Counter
    uint16_t pc;

    // 16-bit Index Register
    uint16_t I;

    // 8-bit Stack Pointer
    uint8_t sp;
    
    // 16-bit Stack
    std::array<uint16_t, 16> stack;

    // 8-bit Delay Timer, decremented at 60Hz
    uint8_t delayTimer;

    // 8-bit Sound Timer, decremented at 60Hz, emits a beep when > 0
    uint8_t soundTimer;

    // 64x32 pixels display
    std::array<uint8_t, 2048> display;

    // Keypad state - 0x0 to 0xF
    std::array<bool, 16> keypad;

    // Current opcode
    uint16_t opcode;

    // The program code
    std::vector<uint8_t> program;

    uint16_t GetNNN();
    uint8_t GetNN();
    uint8_t GetN();
    uint8_t GetX();
    uint8_t GetY();

  public:
    void Initialize();
    void Compile(const char* filename);
    void ReadROM(const char* filename);
    void LoadProgram(std::vector<uint8_t> program);
    void Fetch();
    void Decode();
    void SetKey(uint8_t key);
    void UnsetKey(uint8_t key);
    void DecrementTimers();
    std::array<uint8_t, 2048> GetDisplay();
};
