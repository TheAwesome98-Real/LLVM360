#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct Instruction {
  uint32_t address;
  uint32_t instrWord;
  std::string opcName;

  std::vector<uint32_t> ops;

  inline operator std::string() {
    return opcName;
  }
};
