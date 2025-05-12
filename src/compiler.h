#pragma once

#include <fstream>
#include <iostream>
#include <string>

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
  std::vector<uint8_t> byteCode;

public:
  void Compile(const char *filename)
  {
    std::ifstream infile(filename);
    state = NEW_LINE;
    int lineNo = 1;
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
            case '#':
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
            case '#':
            case '\n': {
              instruction.push_back(mnemonic);
              mnemonic = "";
              if (ch == '\n' || ch == '#') {
                instructions.push_back(instruction);
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
            case '#':
            case '\n':
              instructions.push_back(instruction);
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
              instruction.clear();
              param = "";
              lineNo++;
              state = NEW_LINE;
              break;
            case ' ':
            case '\t':
              break;
            case '#':
              instruction.push_back(param);
              instructions.push_back(instruction);
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
    std::cout << "Generating byte code" << std::endl;
    std::cout << std::endl << "Number of instructions: " << instructions.size() << std::endl;
    for (auto instruction : instructions) {
      auto mnemonic = instruction[0];
      auto params = std::vector<std::string>(instruction.begin() + 1, instruction.end());
      if (mnemonic == "CLS") {
        byteCode.push_back(0x00);
        byteCode.push_back(0x0E);
      } else if (mnemonic == "LD") {
        reg = std::stoi(params.at(0).substr(1, 1), 0, 16);
        value = std::stoi(params.at(1), 0, 16);
        byteCode.push_back(0x60 + reg);
        byteCode.push_back(0x00 + value);
      } else if (mnemonic == "LF") {
        reg = std::stoi(params.at(0).substr(1, 1), 0, 16);
        byteCode.push_back(0xF0 + reg);
        byteCode.push_back(0x29);
      } else if (mnemonic == "DRW") {
        reg = std::stoi(params.at(0).substr(1, 1), 0, 16);
        byteCode.push_back(0xD0 + reg);
        reg = std::stoi(params.at(1).substr(1, 1));
        value = std::stoi(params.at(2), 0, 16);
        byteCode.push_back((reg * 0x10) + value);
      } else if (mnemonic == "JP") {
        value = std::stoi(params.at(0).substr(0, 1), 0, 16);
        byteCode.push_back(0x10 + value);
        value = std::stoi(params.at(0).substr(1, 2), 0, 16);
        byteCode.push_back(value);
      }
    }

    return byteCode;
  }
};
