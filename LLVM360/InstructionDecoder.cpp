#include "InstructionDecoder.h"



// Instruction Bit Field
class IBF
{
public:

	IBF(const uint32_t instr) : m_instr(instr)
	{
	}

	uint32_t GetAt(uint32_t bestart, const uint32_t length, const bool signExtend = false, const int shift = 0)
	{
		uint32_t start = (32 - bestart) - length;
		const uint32_t mask = ((1 << length) - 1);
		uint32_t bitSize = length;
		uint32_t ext = (m_instr >> start) & mask;

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
		return ext;
	}

	inline const uint32_t GetInstr() const
	{
		return m_instr;
	}


private:
	uint32_t m_instr = 0;
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





static inline void SetInstr(Instruction& inst, std::string name, const std::vector<uint32_t>& operands)
{
	inst.opcName = name;
	inst.ops = operands; // Use the raw pointer if required
}

#define INST(name, ...) { SetInstr(instruction, name, std::vector<uint32_t>{ __VA_ARGS__ }); return 4; }


// https://www.nxp.com/docs/en/user-guide/MPCFPE_AD_R1.pdf
// 
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
	instruction.address = m_imageBaseAddress + (stride - m_imageDataPtr);
	instruction.instrWord = instr;
	IBF* ibf = new IBF(instr);

	uint32_t opcode = ibf->GetAt(0, 6); 
	uint32_t S = ibf->GetAt(6, 5);
	uint32_t aa = ibf->GetAt(30, 1);
	uint32_t lk = ibf->GetAt(31, 1);
	uint32_t spr10 = ibf->GetAt(11, 10);
	uint32_t spr5 = ibf->GetAt(11, 5);
	uint32_t li = ibf->GetAt(6, 24, true, 2);

	uint32_t rA = ibf->GetAt(11, 5);
	uint32_t d = ibf->GetAt(16, 16);

	
	
	// The funny giant switch statement instruction decoder
	switch (opcode)
	{
		case 0:

		// li and lis are just addi and addis instructions
		case 14: 
		{
			if (rA == 0) INST("li", S, d) // addi rD,0,SIMM 
			INST("addi", S, rA, d) // addi rD,rA,SIMM 
		}

		case 15:
		{
			if(rA == 0) INST("lis", S, d) // addis rD,0,SIMM
			INST("addis", S, rA, d)	// addis rD,rA,SIMM 
		}

		case 18:
			if (aa == 0 && lk == 1) INST("bl", li)


		case 31:
		{
			uint32_t extOC = ibf->GetAt(21, 10);
			switch (extOC)
			{
				case 339: INST("mfspr", S, spr5) // mfspr rD, spr
				
				
			}
		}
			
		case 37: INST("stfd", S, rA, d) // stwu rS,d(rA)
		case 54: INST("stfd", S, rA, d) // stfd frS, d(rA)


		
		
	}
	
	printf("UNABLE TO DECODE INSTRUCTION: opcode: %i\n", opcode);
	__debugbreak();
	printf("Unknown instruction %08X\n", instr);
		
	return 0;
}


