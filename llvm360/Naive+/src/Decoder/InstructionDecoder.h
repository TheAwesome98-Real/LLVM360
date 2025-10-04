#pragma once
#include <cstdint>
#include "Instruction.h"
#include "Loader/ImageLoader.h"

static inline uint32_t SwapInstrBytes(const uint32_t v) {
  uint32_t ret;
  const uint8_t *p = (const uint8_t *)&v;
  uint8_t *q = (uint8_t *)&ret;
  q[0] = p[3];
  q[1] = p[2];
  q[2] = p[1];
  q[3] = p[0];
  return ret;
}

class InstructionDecoder {
public:
  InstructionDecoder(XLoader::Section* imageSection, const uint8_t* secDataPtr);
  uint32_t GetInstructionAt(uint32_t address, Instruction &instruction);
  uint32_t DecodeInstruction(const uint8_t *stride, Instruction &instruction);

  uint64_t m_imageBaseAddress;
  uint64_t m_imageDataSize;
  const uint8_t *m_imageDataPtr;
};
