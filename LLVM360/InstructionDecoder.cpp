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
	uint32_t ext21_10 = ibf->GetAt(21, 10);
	uint32_t ext22_9 = ibf->GetAt(22, 9);
	uint32_t S = ibf->GetAt(6, 5);
	uint32_t aa = ibf->GetAt(30, 1);
	uint32_t b31_1 = ibf->GetAt(31, 1);
	uint32_t spr10 = ibf->GetAt(11, 10);
	uint32_t spr5 = ibf->GetAt(11, 5);
	uint32_t li = ibf->GetAt(6, 24, true, 2);

	uint32_t b11_5 = ibf->GetAt(11, 5);
	uint32_t b16_16 = ibf->GetAt(16, 16);
	uint32_t b16_14 = ibf->GetAt(16, 14);
	uint32_t b16_5 = ibf->GetAt(16, 5);
	uint32_t b6_3 = ibf->GetAt(6, 3);
	uint32_t b21_5 = ibf->GetAt(21, 5);
	uint32_t b21_1 = ibf->GetAt(21, 1);
	
	
	// The funny giant switch statement instruction decoder
	switch (opcode)
	{

		case 0: INST("nop")

		// VMX fun
		case 4:
		{
			uint32_t v21_11 = ibf->GetAt(21, 11);
			uint32_t v26_6 = ibf->GetAt(26, 6);
			uint32_t v21_7 = ibf->GetAt(21, 7);

			switch (v21_11)
			{
				case 199: INST("stvewx", S, b11_5, b16_5) // stvewx vS,rA,rB
			}

			// extended
			switch (v21_7)
			{
				//stvx vS,rA,rB
				case 28: INST("stvx128", S, b11_5, b16_5) // stvx128 vS,rA,rB
			}


			printf("VMX OPS NOT IMPLEMENTED V21=%d, V26=%d, V21_7=%d opcode: %d\n",
				v21_11, v26_6, v21_7, opcode);
			return 0;
		}





		case 10:
		{
			uint32_t l = ibf->GetAt(10, 1);
			if (!l) INST("cmplwi", b6_3, b11_5, b16_16)
			INST("cmpldi", b6_3, b11_5, b16_16)
		}



		// li and lis are just addi and addis instructions
		case 14: 
		{
			if (b11_5 == 0) INST("li", S, b16_16) // addi rD,0,SIMM 
			INST("addi", S, b11_5, b16_16) // addi rD,rA,SIMM 
		}

		case 15:
		{
			if(b11_5 == 0) INST("lis", S, b16_16) // addis rD,0,SIMM
			INST("addis", S, b11_5, b16_16)	// addis rD,rA,SIMM 
		}

		// bc, bca, bcl, bcla (BO,BI,target_addr)
		case 16:
			if (!aa && !b31_1)	INST("bc", S, b11_5, b16_14);
			if (!aa && b31_1)  INST("bcl", S, b11_5, b16_14);
			if (aa && !b31_1)  INST("bca", S, b11_5, b16_14);
			INST("bcla", S, b11_5, b16_14);

		// b,ba,bl,bla
		case 18:
			if (!aa && !b31_1)	INST("b", li);
			if (!aa && b31_1) INST("bl", li)
			if (aa && !b31_1)  INST("ba", li);
			INST("bla", li)

		case 31:
		{
			switch (ext21_10)
			{
				case 339: INST("mfspr", S, spr5) // mfspr rD, spr
				
				// or rA,rS,rB (also RC)
				case 444:
					if (b31_1) INST("orRC", S, b11_5, b16_5)
					INST("or", S, b11_5, b16_5)

				
			}

			// math extended
			switch (ext22_9)
			{
				// add rD,rA,rB (also RC OE)
				case 266:
					if (!b21_1 && !b31_1)	INST("add", S, b11_5, b16_5);
					if (!b21_1 && b31_1)	INST("addRC", S, b11_5, b16_5);
					if (b21_1 && !b31_1)	INST("addOE", S, b11_5, b16_5);
					if (b21_1 && b31_1)		INST("addOERC", S, b11_5, b16_5);

				// subf rD,rA,rB (also RC OE)
				case 40:
					if (!b21_1 && !b31_1)	INST("subf", S, b11_5, b16_5);
					if (!b21_1 && b31_1)	INST("subfRC", S, b11_5, b16_5);
					if (b21_1 && !b31_1)	INST("subfOE", S, b11_5, b16_5);
					if (b21_1 && b31_1)		INST("subfOERC", S, b11_5, b16_5);

			
			}

			printf("EXTENDED OPS NOT IMPLEMENTED EXTOC21: %i EXTOC21: %i OP: %i\n", ext21_10, ext22_9, opcode);
			return 0;
		}
		
		case 36: INST("stw", S, b11_5, b16_16) // stw rS,d(rA)
		case 37: INST("stwu", S, b11_5, b16_16) // stwu rS,d(rA)

		case 48: INST("lfs", S, b11_5, b16_16) // lfs frD,d(rA)
		case 50: INST("lfd", S, b11_5, b16_16) // lfd frD, d(rA)
		
		case 52: INST("stfs", S, b11_5, b16_16) // stfs frS,d(rA)
		case 54: INST("stfd", S, b11_5, b16_16) // stfd frS, d(rA)


		// offset is *4
		case 58:
			if (aa && !b31_1) INST("lwa", S, b11_5, b16_14) // lwa rD,ds(rA)
			if (!aa && !b31_1) INST("ld", S, b11_5, b16_14) // ld rD,ds(rA)
			if (!aa && b31_1) INST("ldu", S, b11_5, b16_14) // ldu rD,ds(rA)

		case 62: 
			if(!aa && !b31_1) INST("std", S, b11_5, b16_14) // std rS,ds(rA)
			if (!aa && b31_1) INST("stdu", S, b11_5, b16_14) // stdu rS,ds(rA) if lk == 1


		// single floating math wowoy
		case 59:
		{
			switch (ext21_10)
			{
				// fmuls frD,frA,frC (also RC)
				case 25:
					if (b31_1) INST("fmulsRC", S, b11_5, b21_5)
					INST("fmuls", S, b11_5, b21_5)
			}

			printf("EXTENDED SINGLE FLOATING OPS NOT IMPLEMENTED EXTOC: %i OP: %i\n", ext21_10, opcode);
			return 0;
		}



		// double floating math wowoy
		case 63:
		{
			switch (ext21_10)
			{
				// frsp frD,frB (also RC)
				case 12:
					if (b31_1) INST("frspRC", S, b16_5)
					INST("frsp", S, b16_5)

				// fneg frD,frB (also RC)
				case 40:
					if (b31_1) INST("fnegRC", S, b16_5)
					INST("fneg", S, b16_5)

				// fmr frD, frB (also RC)
				case 72:
					if(b31_1) INST("fmrRC", S, b16_5)
					INST("fmr", S, b16_5)

				// fcfid frD,frB (also RC)
				case 846:
					if (b31_1) INST("fcfidRC", S, b16_5)
					INST("fcfid", S, b16_5)
			}

			printf("EXTENDED DOUBLE FLOATING OPS NOT IMPLEMENTED EXTOC: %i OP: %i\n", ext21_10, opcode);
			return 0;
		}
		
	}
	
	printf("UNABLE TO DECODE INSTRUCTION: opcode: %i\n", opcode);
	__debugbreak();
	printf("Unknown instruction %08X\n", instr);
		
	return 0;
}


