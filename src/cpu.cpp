#include <stdexcept>
#include <ios>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <iostream>
#include "cpu.h"

std::array<uint8_t, FONTSET_SIZE> fontset =
{
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

uint16_t CPU::GetNNN()
{
  return opcode & 0x0FFFu;
}

uint8_t CPU::GetNN()
{
  return opcode & 0x00FFu;
}

uint8_t CPU::GetN()
{
  return opcode & 0x000Fu;
}

uint8_t CPU::GetX()
{
  return (opcode & 0x0F00u) >> 8u;
}

uint8_t CPU::GetY()
{
  return (opcode & 0x00F0u) >> 4u;
}

uint16_t GetXYIndex(uint8_t x, uint8_t y) {
  return SCREEN_WIDTH * y + x;
}

void CPU::Initialize()
{
  V.fill(0);
  stack.fill(0);
  display.fill(0);
  keypad.fill(false);
  memory.fill(0);
  I = 0;
  sp = 0;
  opcode = 0;

  // Set fontset in memory
  for (uint8_t i = 0; i < FONTSET_SIZE; i++) {
    memory[i + FONT_START_ADDR] = fontset.at(i);
  }

  pc = PROG_START_ADDR;
}

void CPU::ReadROM(const char *filename)
{
  std::ifstream infile(filename, std::ios::binary);

  uint8_t byte;
  infile.seekg (0, infile.end);
  int length = infile.tellg();
  infile.seekg (0, infile.beg);

  for (uint8_t i = 0; i < length; i++) {
    memory[PROG_START_ADDR + i] = infile.get();
  }

  infile.close();
}

void CPU::LoadProgram(std::vector<uint8_t> byteCode)
{
  uint8_t idx = 0;
  for (auto byte : byteCode) {
    memory[PROG_START_ADDR + idx] = byte;
    idx++;
  }
}

void CPU::Fetch()
{
  uint8_t hi = memory[pc++];
  uint8_t lo = memory[pc++];
  opcode = hi << 8 | lo;
}

void CPU::Decode()
{
  switch (opcode & 0xF000) {
    case 0x0000:
      switch (opcode & 0x000F) {
        case 0x0000:  // 0x00E0 - CLS - Clear Screen
          display.fill(0);
          break;
        case 0x000E:  // 0x00EE - RTS
          break;
        default:
          printf ("Unknown opcode [0x0000]: 0x%X\n", opcode);
      }
      break;
    case 0x1000:  // 0x1NNN - JP NNN
      pc = GetNNN();
      break;
    case 0x2000:  // 0x2NNN - CALL NNN
      if (sp == stack.size()) {
        printf("Stack out of space!");
        break;
      }
      stack[sp++] = pc;
      pc = GetNNN();
      break;
    case 0x6000:  // 0x6XNN - LD Vx, NN
      V[GetX()] = GetNN();
      break;
    case 0x7000:  // 0x7XNN - ADD Vx, NN
      V[GetX()] += GetNN();
      break;
    case 0x8000:
      switch (opcode & 0x000E) {
        case 0x0000:  // 0x8XY0 - LD Vx, Vy
          V[GetX()] = V[GetY()];
          break;
        case 0x0001:  // OR
          V[GetX()] |= V[GetY()];
          break;
        case 0x0002:  // AND
          V[GetX()] &= V[GetY()];
          break;
        case 0x003:   // XOR
          V[GetX()] ^= V[GetY()];
          break;
      }
      break;
    case 0xA000:  // 0xANNN - LD I, NNN
      I = GetNNN();
      break;
    case 0xD000: {  // 0xDXYN - DRW X, Y, N
      uint8_t height = GetN();
      uint8_t xPos = V[GetX()] % SCREEN_WIDTH;
      uint8_t yPos = V[GetY()] % SCREEN_HEIGHT;
      V[0xF] = 0;

      for (uint8_t row = 0; row < height; row++) {
        uint8_t spriteByte = memory[I + row];
        for (uint8_t col = 0; col < 8; col++) {
          uint8_t spritePixel = spriteByte & (0x80u >> col);
          uint16_t pos = GetXYIndex(xPos + col, yPos + row);
          if (spritePixel) {
            if (display[pos]) {
              V[0xF] = 1;
            }
            display[pos] ^= 1;
          }
        }
      }
      break;
    }
    case 0xF000:
      switch (opcode & 0x00FF) {
        case 0x0007: {  // FX07 - LD Vx, DT
          V[GetX()] = delayTimer;
          break;
        }
        case 0x0015: {  // FX15 - LD DT, Vx
          delayTimer = V[GetX()];
          break;
        }
        case 0x0018: {  // FX18 - LD ST, Vx
          soundTimer = V[GetX()];
          break;
        }
        case 0x000A: {  // LD Vx, K
          bool isWaiting = true;
          for (uint8_t i = 0; i < keypad.size(); i++) {
            if (keypad.at(i)) {
              V[GetX()] = i;
              isWaiting = false;
              break;
            }
          }
          if (isWaiting) {
            pc -= 2;
          }
          break;
        }
        case 0x001E: {  // ADD I, Vx
          I += V[GetX()];
          // TODO: Check for overflow, set V[F] accordingly
          break;
        }
        case 0x0029: {  // FX29 - LD F, Vx
          uint8_t digit = V[GetX()];
          I = FONT_START_ADDR + (5 * digit);
          break;
        }
      }
      break;
    default: 
      std::stringstream ss;
      ss << "No such opcode 0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << opcode << " at address 0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << pc - 2;
      throw std::invalid_argument(ss.str());
  }
}

void CPU::SetKey(uint8_t key)
{
  keypad[key] = true;
}

void CPU::UnsetKey(uint8_t key)
{
  keypad[key] = false;
}

void CPU::DecrementTimers() {
  if (delayTimer > 0) {
    delayTimer -= 1;
  }

  if (soundTimer > 0) {
    soundTimer -= 1;
  }
}

std::array<uint8_t, 2048> CPU::GetDisplay()
{
  return display;
}
