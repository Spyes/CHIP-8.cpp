#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

enum State
{
  NEW_LINE,
  MNEMONIC,
  NEW_PARAM,
  PARAM,
  IGNORE,
};

typedef std::vector<std::string> Instruction;

class Compiler
{
private:
  State state;
  std::vector<Instruction> instructions;
  std::unordered_map<std::string, uint16_t> labels;
  std::vector<uint8_t> byteCode;

  int GetReg(std::string param)
  {
    return std::stoi(param.substr(1, 1), 0, 16);
  }

  bool IsRegister(std::string param)
  {
    return param.substr(0, 1) == "$";
  }

public:
  void Compile(const char *filename)
  {
    std::ifstream infile(filename);
    state = NEW_LINE;
    int lineNo = 1;
    uint16_t pc = 0x0200; // TODO: Get from settings
    char ch;
    std::string mnemonic;
    std::string param;
    Instruction instruction;
    while (infile >> std::noskipws >> ch) {
      switch (state) {
        case IGNORE: {
          switch (ch) {
            case '\n':
              state = NEW_LINE;
              lineNo++;
              break;
            default:
              break;
          }
          break;
        }
        case NEW_LINE: {
          switch (ch) {
            case ' ':
            case '\t':
              break;
            case ';':
              state = IGNORE;
              break;
            case '\n':
              lineNo++;
              break;
            default:
              mnemonic += ch;
              state = MNEMONIC;
              break;
          }
          break;
        }
        case MNEMONIC: {
          switch (ch) {
            case ' ':
            case '\t':
            case ';':
            case '\n': {
              instruction.push_back(mnemonic);
              mnemonic = "";
              if (ch == '\n' || ch == ';') {
                instructions.push_back(instruction);
                pc += 2;
                instruction.clear();
                if (ch == '\n') {
                  lineNo++;
                  state = NEW_LINE;
                } else {
                  state = IGNORE;
                }
              } else {
                state = NEW_PARAM;
              }
              break;
            }
            case ':': {
              labels[mnemonic] = pc;
              mnemonic = "";
              break;
            }
            default:
              mnemonic += ch;
              break;
          }
          break;
        }
        case NEW_PARAM: {
          switch (ch) {
            case ' ':
            case '\t':
              break;
            case ';':
            case '\n':
              instructions.push_back(instruction);
              pc += 2;
              instruction.clear();
              if (ch == '\n') {
                lineNo++;
                state = NEW_LINE;
              } else {
                state = IGNORE;
              }
              break;
            default:
              param += ch;
              state = PARAM;
              break;
          }
          break;
        }
        case PARAM: {
          switch (ch) {
            case '\n':
              instruction.push_back(param);
              instructions.push_back(instruction);
              pc += 2;
              instruction.clear();
              param = "";
              lineNo++;
              state = NEW_LINE;
              break;
            case ' ':
            case '\t':
              break;
            case ';':
              instruction.push_back(param);
              instructions.push_back(instruction);
              pc += 2;
              instruction.clear();
              param = "";
              state = IGNORE;
              break;
            case ',':
              instruction.push_back(param);
              param = "";
              break;
            default:
              param += ch;
              break;
          }
          break;
        }
      }
    }
    infile.close();
  }

  std::vector<uint8_t> Generator() {
    uint8_t reg;
    uint8_t value;
    uint16_t addr;
    std::cout << "Generating byte code" << std::endl;
    std::cout << std::endl << "Number of instructions: " << instructions.size() << std::endl;
    for (auto instruction : instructions) {
      auto mnemonic = instruction[0];
      auto params = std::vector<std::string>(instruction.begin() + 1, instruction.end());
      if (mnemonic == "CLS") {
        byteCode.push_back(0x00);
        byteCode.push_back(0x0E);
      } else if (mnemonic == "LD") {
        if (params.at(1) == "DT") {
          reg = GetReg(params.at(0));
          byteCode.push_back(0xF0 + reg);
          byteCode.push_back(0x07);
        } else if (params.at(0) == "DT") {
          reg = GetReg(params.at(1));
          byteCode.push_back(0xF0 + reg);
          byteCode.push_back(0x15);
        } else if (params.at(0) == "ST") {
          reg = GetReg(params.at(1));
          byteCode.push_back(0xF0 + reg);
          byteCode.push_back(0x18);
        } else if (params.at(1) == "K") {
          reg = GetReg(params.at(0));
          byteCode.push_back(0xF0 + reg);
          byteCode.push_back(0x0A);
        } else if (params.at(0) == "I") {
          if (IsRegister(params.at(1))) {
            reg = GetReg(params.at(1));
            byteCode.push_back(0xF0 + reg);
            byteCode.push_back(0x29);
          } else {
            if (addr = labels[params.at(0)]) {
              byteCode.push_back(0xA0 + (addr >> 8u));
              byteCode.push_back(addr & 0x00FFu);
            } else {
              value = std::stoi(params.at(0).substr(0, 1), 0, 16);
              byteCode.push_back(0xA0 + value);
              value = std::stoi(params.at(0).substr(1, 2), 0, 16);
              byteCode.push_back(value);
            }
          }
        } else if (IsRegister(params.at(0)) && IsRegister(params.at(1))) {
          byteCode.push_back(0x80 + GetReg(params.at(0)));
          byteCode.push_back(GetReg(params.at(0)) * 0x10);
        } else {
          reg = GetReg(params.at(0));
          value = std::stoi(params.at(1), 0, 16);
          byteCode.push_back(0x60 + reg);
          byteCode.push_back(0x00 + value);
        }
      } else if (mnemonic == "DRW") {
        reg = GetReg(params.at(0));
        byteCode.push_back(0xD0 + reg);
        reg = GetReg(params.at(1));
        value = std::stoi(params.at(2), 0, 16);
        byteCode.push_back((reg * 0x10) + value);
      } else if (mnemonic == "JP") {
        if (addr = labels[params.at(0)]) {
          byteCode.push_back(0x10 + (addr >> 8u));
          byteCode.push_back(addr & 0x00FFu);
        } else {
          value = std::stoi(params.at(0).substr(0, 1), 0, 16);
          byteCode.push_back(0x10 + value);
          value = std::stoi(params.at(0).substr(1, 2), 0, 16);
          byteCode.push_back(value);
        }
      } else if (mnemonic == "ADD") {
        if (params.at(0) == "I") {
          reg = GetReg(params.at(1));
          byteCode.push_back(0xF0 + reg);
          byteCode.push_back(0x1E);
        } else {   // ADD Vx, NN
          reg = GetReg(params.at(0));
          value = std::stoi(params.at(1), 0, 16);
          byteCode.push_back(0x70 + reg);
          byteCode.push_back(value);
        }
      } else if (mnemonic == "RTS") {
        byteCode.push_back(0x00);
        byteCode.push_back(0xEE);
      } else if (mnemonic == "CALL") {
        if (addr = labels[params.at(0)]) {
          byteCode.push_back(0x20 + (addr >> 8u));
          byteCode.push_back(addr & 0x00FFu);
        } else {
          value = std::stoi(params.at(0).substr(0, 1), 0, 16);
          byteCode.push_back(0x20 + value);
          value = std::stoi(params.at(0).substr(1, 2), 0, 16);
          byteCode.push_back(value);
        }
      } else if (mnemonic == "SE") {
        if (IsRegister(params.at(1))) {
          reg = GetReg(params.at(0));
          byteCode.push_back(0x50 + reg);
          reg = GetReg(params.at(1));
          byteCode.push_back(reg * 0x10);
        } else {
          reg = GetReg(params.at(0));
          value = std::stoi(params.at(1).substr(1, 2), 0, 16);
          byteCode.push_back(0x30 + reg);
          byteCode.push_back(value);
        }
      } else if (mnemonic == "SNE") {
        if (IsRegister(params.at(1))) {
          reg = GetReg(params.at(0));
          byteCode.push_back(0x90 + reg);
          reg = GetReg(params.at(1));
          byteCode.push_back(reg * 0x10);
        } else {
          reg = GetReg(params.at(0));
          value = std::stoi(params.at(1).substr(1, 2), 0, 16);
          byteCode.push_back(0x40 + reg);
          byteCode.push_back(value);
        }
      } else if (mnemonic == "OR" || mnemonic == "AND" || mnemonic == "XOR") {
        if (IsRegister(params.at(0)) && IsRegister(params.at(1))) {
          uint8_t identifier;
          if (mnemonic == "OR") identifier = 0x01;
          else if (mnemonic == "AND") identifier = 0x02;
          else if (mnemonic == "XOR") identifier = 0x03;
          else {
            std::cout << "ERROR: No such command " << mnemonic << std::endl;
            return;
          }
          byteCode.push_back(0x80 + GetReg(params.at(0)));
          byteCode.push_back((GetReg(params.at(1)) * 0x10) + identifier);
        }
      }
    }

    return byteCode;
  }
};
