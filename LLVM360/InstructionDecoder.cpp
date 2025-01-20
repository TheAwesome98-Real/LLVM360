#include "InstructionDecoder.h"


template< const uint32_t bestart, const uint32_t length, const bool signExtend = false, const int shift = 0 >
class BitField
{
public:
	static const uint32_t start = (32 - bestart) - length;

	BitField(const uint32_t input)
	{
		const uint32_t mask = ((1 << length) - 1);
		uint32_t bitSize = length;
		uint32_t ext = (input >> start) & mask;

		// additional left shift
		if (shift > 0)
		{
			ext <<= shift;
			bitSize += shift;
		}

		// if signed expand copy the sign bit
		if (signExtend)
		{
			const uint32_t signBitValue = ext & (1 << (bitSize - 1));
			if (signBitValue != 0)
			{
				const uint32_t signMask = ((uint32_t)-1) << bitSize;
				ext |= signMask;
			}
		}

		// right shift (always do that after sign conversion)
		if (shift < 0)
		{
			ext >>= (-shift);
		}

		// save value
		m_val = ext;
	}

	inline const uint32_t Get() const
	{
		return m_val;
	}

	inline operator uint32_t() const
	{
		return m_val;
	}

private:
	uint32_t m_val;
};


InstructionDecoder::InstructionDecoder(const Section* imageSection)
{
	m_imageSection = imageSection;
	m_imageBaseAddress = imageSection->GetVirtualOffset();
	m_imageDataSize = imageSection->GetVirtualSize();
	m_imageDataPtr = (const uint8_t*)imageSection->GetImage()->GetMemory() + (m_imageBaseAddress - imageSection->GetImage()->GetBaseAddress());
}

uint32_t InstructionDecoder::GetInstructionAt(uint32_t address, Instruction& instruction)
{
	// build input stream
	const uint64_t offset = address - m_imageBaseAddress;
	const uint8_t* stride = m_imageDataPtr + offset;

	return DecodeInstruction(stride, instruction);
}

// create macro to instantiate a new Instruction object, so the macro takes as parameters name, that is a string that will set the opcName field of the instruction, instruction, that is the reference to set, and a series of parameters that are the operands of the instruction, the instruction constructor takes several operands as parameter (not all of them used) so set those operands with the bitfield class
#define INST(name, instruction, ...) instruction.opcName = name; instruction.completeInstr = *(uint32_t*)stride; __VA_ARGS__;



// *Stride* pointer to start of the 4 bytes of the instruction in memory
// [out] instruction - the decoded instruction
uint32_t InstructionDecoder::DecodeInstruction(const uint8_t* stride, Instruction& instruction)
{
	// Xenons PowerPC instructions are always 4 bytes long, 
	// this helps us to avoid instruction length decoding since 
	// we already know the length
	// we can store it in a uint32 and use bitfields to extract the opcode and operands

	// cause how it is stored in the stride we have to swap endians
	uint32_t instr = SwapInstrBytes(*(uint32_t*)stride);
	BitField<0, 6> opcode(instr);
	
	
	// The funny giant switch statement instruction decoder
	switch (opcode.Get())
	{
		case 0:



		case 31:
			BitField<21, 10> extOC(instr);
			switch (extOC.Get())
			{
			case 339: INST("mfspr", instruction) break;

			}

		
	}
	return 4;
}


