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
	uint32_t b11_10 = ibf->GetAt(11, 10);
	uint32_t spr5 = ibf->GetAt(11, 5);
	uint32_t li = ibf->GetAt(6, 24, true, 2);
	uint32_t ext26_5 = ibf->GetAt(26, 5);

	uint32_t b11_5 = ibf->GetAt(11, 5);
	uint32_t b6_5 = ibf->GetAt(6, 5);
	uint32_t b16_16 = ibf->GetAt(16, 16);
	uint32_t b16_14 = ibf->GetAt(16, 14);
	uint32_t b16_5 = ibf->GetAt(16, 5);
	uint32_t b6_3 = ibf->GetAt(6, 3);
	uint32_t b21_5 = ibf->GetAt(21, 5);
	uint32_t b21_1 = ibf->GetAt(21, 1);
	uint32_t b26_5 = ibf->GetAt(26, 5);


	uint32_t vshift28 = ibf->GetAt(28, 2, false, 5);
	uint32_t vshift30 = ibf->GetAt(30, 2, false, 5);
	uint32_t vshift26a = ibf->GetAt(21, 1, false, 6);
	uint32_t vshift26b = ibf->GetAt(26, 1, false, 5);


	// extended regs (full 128 regs)
	uint32_t vreg6 = vshift28 + b6_5;
	uint32_t vreg11 = vshift26a + vshift26b + b11_5;
	uint32_t vreg16 = vshift30 + b16_5;
	
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
			
				case 1156:
				{
					if (b11_5 == b16_5)
					{
						INST("mr", b6_5, b11_5);
					}
					else
					{
						INST("vor", b6_5, b11_5, b16_5);
					}
				}
			}

			// extended
			switch (v21_7)
			{
				case 12: INST("lvx", vreg6, b11_5, b16_5);
				//stvx vS,rA,rB
				case 28: INST("stvx128", S, b11_5, b16_5) // stvx128 vS,rA,rB
			}

			switch (v26_6)
			{
			case 46: INST("vmaddfp", b6_5, b11_5, b21_5, b16_5);
			case 47: INST("vnmsubfp", b6_5, b11_5, b16_5, b21_5);
			}



			printf("VMX OPS NOT IMPLEMENTED V21=%d, V26=%d, V21_7=%d opcode: %d\n",
				v21_11, v26_6, v21_7, opcode);
			return 0;
		}


		// EXT VMX
		case 5:
		{
			uint32_t v22_4 = ibf->GetAt(22, 4);
			uint32_t v27_1 = ibf->GetAt(27, 1);

			if (v27_1 == 0)
			{
				
				switch (v22_4)
				{

				}
			}
			else
			{

				switch (v22_4)
				{

					case 0: INST("vaddfp", vreg6, vreg11, vreg16);
					case 1: INST("vsubfp", vreg6, vreg11, vreg16);
					case 2: INST("vmulfp128", vreg6, vreg11, vreg16);

					case 3: INST("vmaddfp", vreg6, vreg11, vreg16, vreg6);
					case 4: INST("vmaddfp", vreg6, vreg11, vreg6, vreg16); // addc
					
					case 5: INST("vnmsubfp", vreg6, vreg11, vreg16, vreg6);

					case 6: INST("vdot3fp", vreg6, vreg11, vreg16);
					case 7: INST("vdot4fp", vreg6, vreg11, vreg16);

					case 8: INST("vand", vreg6, vreg11, vreg16);
					case 9: INST("vandc", vreg6, vreg11, vreg16);
					case 10: INST("vnor", vreg6, vreg11, vreg16);
					case 12: INST("vxor", vreg6, vreg11, vreg16);
					case 13: INST("vsel", vreg6, vreg11, vreg16, vreg6); // arg3 == arg0
					case 11:
					{
						if (vreg11 == vreg16)
						{
							INST("mr", vreg6, vreg11);
						}
						else
						{



							INST("vor", vreg6, vreg11, vreg16);
						}
					}
				}
			}


			printf("VMX OPS NOT IMPLEMENTED V22=%d, V27=%d, opcode: %d\n",
				v22_4, v27_1, opcode);
			return 0;
		}

		// EXT VMX
		case 6:
		{
			uint32_t v21_11 = ibf->GetAt(21, 11);
			uint32_t v26_6 = ibf->GetAt(26, 6);
			uint32_t v21_7 = ibf->GetAt(21, 7);
			uint32_t v22_4 = ibf->GetAt(22, 4);
			uint32_t v27_1 = ibf->GetAt(27, 1);

			switch (v21_7)
			{	

				case (33 + (0 * 4)): INST("vpermwi128", vreg6, vreg16, b11_5 + 0 * 32);
				case (33 + (1 * 4)): INST("vpermwi128", vreg6, vreg16, b11_5 + 1 * 32);
				case (33 + (2 * 4)): INST("vpermwi128", vreg6, vreg16, b11_5 + 2 * 32);
				case (33 + (3 * 4)): INST("vpermwi128", vreg6, vreg16, b11_5 + 3 * 32);
				case (33 + (4 * 4)): INST("vpermwi128", vreg6, vreg16, b11_5 + 4 * 32);
				case (33 + (5 * 4)): INST("vpermwi128", vreg6, vreg16, b11_5 + 5 * 32);
				case (33 + (6 * 4)): INST("vpermwi128", vreg6, vreg16, b11_5 + 6 * 32);
				case (33 + (7 * 4)): INST("vpermwi128", vreg6, vreg16, b11_5 + 7 * 32);



				// TODO, make those right 
				// vspltisw128
				case 119: INST("vspltisw", vreg6, b11_5) // 128?
				case 43: INST("vcsxwfp", vreg6, vreg16, b11_5)

				case 103: INST("vrsqrtefp", vreg6, vreg16)
			}

			if (v27_1 == 0)
			{
				switch (v22_4)
				{
					case 0: INST("vcmpeqfp", vreg6, vreg11, vreg16)
					case 1: INST("vcmpeqfpRC", vreg6, vreg11, vreg16)
					case 2: INST("vcmpgefp", vreg6, vreg11, vreg16)
					case 3: INST("vcmpgefpRC", vreg6, vreg11, vreg16)
					case 4: INST("vcmpgtfp", vreg6, vreg11, vreg16)
					case 5: INST("vcmpgtfpRC", vreg6, vreg11, vreg16)
					case 6: INST("vcmpbfp", vreg6, vreg11, vreg16)
					case 7: INST("vcmpbfpRC", vreg6, vreg11, vreg16)
					case 8: INST("vcmpequw", vreg6, vreg11, vreg16)
					case 9: INST("vcmpequwRC", vreg6, vreg11, vreg16)
					case 10: INST("vmaxfp", vreg6, vreg11, vreg16)
					case 11: INST("vminfp", vreg6, vreg11, vreg16)
					case 12: INST("vmrghw", vreg6, vreg11, vreg16)
					case 13: INST("vmrglw", vreg6, vreg11, vreg16)
				
				}
			}
			else
			{
				switch (v22_4)
				{
				case 1: INST("vrlw", vreg6, vreg11, vreg16);

				case 3: INST("vslw", vreg6, vreg11, vreg16);

				case 5: INST("vsraw", vreg6, vreg11, vreg16);

				case 7: INST("vsrw", vreg6, vreg11, vreg16);

				case 12: INST("vrfim", vreg6, vreg16);
				case 13: INST("vrfin", vreg6, vreg16);
				case 14: INST("vrfip", vreg6, vreg16);
				case 15: INST("vrfiz", vreg6, vreg16);
				}
			}


			printf("VMX OPS NOT IMPLEMENTED V22_4=%d, V27_1=%d, V21_7=%d opcode: %d\n",
				v22_4, v27_1, v21_7, opcode);
			return 0;
		}

		// muli (multiply with immediate)
		case 7: INST("mulli", S, b11_5, b16_16);

			// subf( subtract from)
		case 8: INST("subfic", S, b11_5, b16_16);



		case 10:
		{
			uint32_t l = ibf->GetAt(10, 1);
			if (!l) INST("cmplwi", b6_3, b11_5, b16_16)
			INST("cmpldi", b6_3, b11_5, b16_16)
		}

		// cmpi (cmpdi, cmpwi)
		case 11:
		{
			uint32_t l = ibf->GetAt(10, 1);
			if (!l) INST("cmpwi", b6_3, b11_5, b16_16)
			INST("cmpdi", b6_3, b11_5, b16_16)
		}
		case 12: INST("addic", S, b11_5, b16_16); // addic rD,rA,SIMM
		case 13: INST("addicRC", S, b11_5, b16_16); // addic. rD,rA,SIMM
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



		case 19:
		{
				switch (ext21_10)
				{
				case 16:
					if (b31_1)	INST("bclrl", b6_5, b11_5)
					INST("bclr", b6_5, b11_5)

				case 528:
					if (b31_1)	INST("bcctrl", b6_5, b11_5)
					INST("bcctr", b6_5, b11_5)

				}

				printf("EXTENDED OPS NOT IMPLEMENTED EXTOC21: %i EXTOC21_9: %i OP: %i\n", ext21_10, ext22_9, opcode);
				return 0;
		}

		// rlwimi, rlwimi., rlwinm, rlwinm., rlwnm, rlwnm.
		case 20: 
			if (b31_1) INST("rlwimiRC", b11_5, S, b16_5, b21_5, b26_5)
			INST("rlwimi", b11_5, S, b16_5, b21_5, b26_5)

		case 21: 
			if (b31_1) INST("rlwinmRC", b11_5, S, b16_5, b21_5, b26_5)
			INST("rlwinm", b11_5, S, b16_5, b21_5, b26_5)

		case 23:
			if (b31_1) INST("rlwnmRC", b11_5, S, b16_5, b21_5, b26_5)
			INST("rlwnm", b11_5, S, b16_5, b21_5, b26_5)

		case 25: INST("oris", b11_5, S, b16_16);
		case 26: INST("xori", b11_5, S, b16_16);
		case 27: INST("xoris", b11_5, S, b16_16);
		case 28: INST("andiRC", b11_5, S, b16_16);
		case 29: INST("andisRC", b11_5, S, b16_16);


		// ori rA,rS,UIMM
		case 24: INST("ori", b11_5, b6_5, b16_16);

		case 31:
		{
			switch (ext21_10)
			{
				
				// and, and., or, or., xor, xor., nand, nand, nor, nor., eqv, eqv., andc, andc., orc, orc
				case 28:
					if (b31_1) INST("andRC", b11_5, S, b16_5)
					INST("and", b11_5, S, b16_5)

				// or rA,rS,rB (also RC)
				case 444:
					if (b31_1) INST("orRC", b11_5, S, b16_5)
					INST("or", b11_5, S, b16_5)

				case 316:
					if (b31_1) INST("xorRC", b11_5, S, b16_5)
					INST("xor", b11_5, S, b16_5)

				case 476: 
					if (b31_1) INST("nandRC", b11_5, S, b16_5)
					INST("nand", b11_5, S, b16_5)

				case 124: 
					if (b31_1) INST("norRC", b11_5, S, b16_5)
					INST("nor", b11_5, S, b16_5)

				case 284: 
					if (b31_1) INST("eqvRC", b11_5, S, b16_5)
					INST("eqv", b11_5, S, b16_5)

				case 60: 
					if (b31_1) INST("andcRC", b11_5, S, b16_5)
					INST("andc", b11_5, S, b16_5)

				case 412: 
					if (b31_1) INST("orcRC", b11_5, S, b16_5)
					INST("orc", b11_5, S, b16_5)


				

						// mtspr, mfspr (move to/from system register)
				case 467: INST("mtspr", b11_10, b6_5)
				case 339: INST("mfspr", b6_5, b11_10)
			}

			// math extended
			switch (ext22_9)
			{

				

				// subfe, subfe., subfeo, subfeo.
				case 136:
					if (!b21_1 && !b31_1)	INST("subfe", S, b11_5, b16_5);
					if (!b21_1 && b31_1)	INST("subfeRC", S, b11_5, b16_5);
					if (b21_1 && !b31_1)	INST("subfeOE", S, b11_5, b16_5);
					if (b21_1 && b31_1)		INST("subfeOERC", S, b11_5, b16_5);


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
		

		case 32: INST("lwz", S, b16_16, b11_5) // lwz rS,d(rA)
		case 33: INST("lwzu", S, b16_16, b11_5) // lwzu rS,d(rA)
		case 36: INST("stw", S, b16_16, b11_5) // stw rS,d(rA)
		case 37: INST("stwu", S, b16_16, b11_5) // stwu rS,d(rA)
		case 38: INST("stb", S, b16_16, b11_5) // stb rS,d(rA)
		case 39: INST("stbu", S, b16_16, b11_5) // stbu rS,d(rA)
		case 40: INST("lhz", S, b16_16, b11_5) // lhz rS,d(rA)
		case 41: INST("lhzu", S, b16_16, b11_5) // lhzu rS,d(rA)
		case 42: INST("lha", S, b16_16, b11_5) // lha rS,d(rA)
		case 43: INST("lhau", S, b16_16, b11_5) // lhau rS,d(rA)
		case 44: INST("sth", S, b16_16, b11_5) // sth rS,d(rA)
		case 45: INST("sthu", S, b16_16, b11_5) // sthu rS,d(rA)

		case 48: INST("lfs", S, b16_16, b11_5) // lfs frD,d(rA)
		case 49: INST("lfsu", S, b16_16, b11_5); // lfsu frD, d(rA)
		case 50: INST("lfd", S, b16_16, b11_5) // lfd frD, d(rA)
		case 51: INST("lfdu", S, b16_16, b11_5); // lfdu frD, d(rA)
		case 52: INST("stfs", S, b16_16, b11_5) // stfs frS,d(rA)

		case 54: INST("stfd", S, b16_16, b11_5) // stfd frS, d(rA)


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
			switch (ext26_5)
			{
				case 20:
					if (b31_1) INST("fsubsRC", S, b11_5, b16_5)
					INST("fsubs", S, b11_5, b16_5)
				case 21: 
					if (b31_1) INST("faddsRC", S, b11_5, b16_5)
					INST("fadds", S, b11_5, b16_5)
				// fmuls frD,frA,frC (also RC)
				case 25:
					if (b31_1) INST("fmulsRC", S, b11_5, b21_5)
					INST("fmuls", S, b11_5, b21_5)
				case 18: 
					if (b31_1) INST("fdivsRC", S, b11_5, b16_5)
					INST("fdivs", S, b11_5, b16_5)
						// fmadd, fmsub, fnmadd, fnmsub
				case 29:
					if (b31_1) INST("fmaddsRC", S, b11_5, b21_5, b16_5)
					INST("fmadds", S, b11_5, b21_5, b16_5)

				case 28: 
					if (b31_1) INST("fmsubsRC", S, b11_5, b21_5, b16_5)
					INST("fmsubs", S, b11_5, b21_5, b16_5)

				case 31:
					if (b31_1) INST("fnmaddsRC", S, b11_5, b21_5, b16_5)
					INST("fnmadds", S, b11_5, b21_5, b16_5)

				case 30:
					if (b31_1) INST("fnmsubsRC", S, b11_5, b21_5, b16_5)
					INST("fnmsubs", S, b11_5, b21_5, b16_5)

					// fsqrts, fres
				case 22:
					if (b31_1) INST("fsqrtsRC", S, b16_5)
					INST("fsqrts", S, b16_5)

				case 24:
					if (b31_1) INST("fresRC", S, b16_5)
					INST("fres", S, b16_5)

				
				
			}

			printf("EXTENDED SINGLE FLOATING OPS NOT IMPLEMENTED EXTOC26_5: %i OP: %i\n", ext26_5, opcode);
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

				
				case 264: 
					if (b31_1) INST("fabsRC", S, b16_5)
						INST("fabs", S, b16_5)

				case 136:
					if (b31_1) INST("fnabsRC", S, b16_5)
						INST("fnabs", S, b16_5)

				case 815:
					if (b31_1) INST("fctidzRC", S, b16_5)
					INST("fctidz", S, b16_5)

				// fcfid frD,frB (also RC)
				case 846:
					if (b31_1) INST("fcfidRC", S, b16_5)
					INST("fcfid", S, b16_5)
			}

			switch (ext26_5)
			{
				// fadd, fsub, fmul, fdiv
				case 21: 
					if (b31_1) INST("faddRC", S, b11_5, b16_5)
					INST("fadd", S, b11_5, b16_5)

				case 20: 
					if (b31_1) INST("fsubRC", S, b11_5, b16_5)
					INST("fsub", S, b11_5, b16_5)

				case 25:									 // BEWARE, the second arg is at bit 21
					if (b31_1) INST("fmulRC", S, b11_5, b16_5)
					INST("fmul", S, b11_5, b16_5)

				case 18: 
					if (b31_1) INST("fdivRC", S, b11_5, b16_5)
					INST("fdiv", S, b11_5, b16_5)


					// fmadd, fmsub, fnmadd, fnmsub
				case 29:
					if (b31_1) INST("fmaddRC", S, b11_5, b21_5, b16_5)
					INST("fmadd", S, b11_5, b21_5, b16_5)

				case 28: 
					if (b31_1) INST("fmsubRC", S, b11_5, b21_5, b16_5)
					INST("fmsub", S, b11_5, b21_5, b16_5)

				case 31: 
					if (b31_1) INST("fnmaddRC", S, b11_5, b21_5, b16_5)
					INST("fnmadd", S, b11_5, b21_5, b16_5)

				case 30: 
					if (b31_1) INST("fnmsubRC", S, b11_5, b21_5, b16_5)
					INST("fnmsub", S, b11_5, b21_5, b16_5)


				// fsel frD,frA,frC,frB 
				case 23:
					if (b31_1) INST("fselRC", S, b11_5, b21_5, b16_5)
					INST("fsel", S, b11_5, b21_5, b16_5)


					// fsqrt and some other optional instructions
				case 22: 
					if (b31_1) INST("fsqrtRC", S, b16_5)
					INST("fsqrt", S, b16_5)

				case 24: 
					if (b31_1) INST("freRC", S, b16_5)
					INST("fre", S, b16_5)

				case 26: 
					if (b31_1) INST("frsqrteRC", S, b16_5)
					INST("frsqrte", S, b16_5)

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


