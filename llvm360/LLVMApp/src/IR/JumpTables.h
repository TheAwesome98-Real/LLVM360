#pragma once
#include "IRGenerator.h"



static const std::vector<const char*> COMPUTED_TABLE_0_pattern = { "lis", "addi", "lbzx", "rlwinm", "lis", "ori", "addi", "add", "mtspr", "bcctr" };

enum JTVariantEnum
{
    COMPUTED_TABLE_0,
    ENUM_SIZE
};

struct JTVariant
{
    JTVariantEnum type;
    std::vector<const char*> pattern;
};

static const std::array<JTVariant, JTVariantEnum::ENUM_SIZE> jtVariantTypes = {
    JTVariant{COMPUTED_TABLE_0, COMPUTED_TABLE_0_pattern},
};



// Define a jump table inside a function
// `targets` are the addresses of the cases, index 0 is always the default case
class JumpTable
{
public:
    uint32_t start_Address;
    uint32_t end_Address;
    JTVariant variant;
    uint32_t numTargets;
	std::vector<uint32_t> targets;

	void ComputeTargets(IRGenerator* irGen)
    {
		
		findTargetsSize(irGen);
		
        switch (variant.type)
        {
        case COMPUTED_TABLE_0:
        {
			uint32_t off = 0;
            Instruction instr = irGen->instrsList.at(start_Address);
            off = instr.ops[2] << 16;
            instr = irGen->instrsList.at(start_Address + 4);
            off += instr.ops[2];
            
            
            
            Section* sec = irGen->m_xexImage->getSectionByAddressBounds(off);
            const uint8_t* m_imageDataPtr = (const uint8_t*)sec->GetImage()->GetMemory() + (sec->GetVirtualOffset() - sec->GetImage()->GetBaseAddress());
            const auto* offsets = (uint8_t*)m_imageDataPtr + off - sec->GetVirtualOffset();

            instr = irGen->instrsList.at(start_Address + (4 * 4));  // lis
            uint32_t baseAddr = instr.ops[2] << 16; 
            instr = irGen->instrsList.at(start_Address + (6 * 4));  // addi
            baseAddr += instr.ops[2]; 
            instr = irGen->instrsList.at(start_Address + (3 * 4));  // rlwinm
            uint32_t sh = instr.ops[2]; 

            for (size_t i = 1; i < numTargets; i++)
            {
                targets.push_back(baseAddr + (offsets[i] << sh));
            }
            break;
        }
		
		break;

        default:
            break;
        }
    }

private:

    void findTargetsSize(IRGenerator* irGen)
    {
        uint32_t field = -1;
        for (size_t i = 0; i < 10; i++)
        {
            Instruction instr = irGen->instrsList.at(start_Address - (i * 4));
            
            if (strcmp(instr.opcName.c_str(), "bc") == 0 || strcmp(instr.opcName.c_str(), "bca") == 0)
            {
                field = instr.ops[1] / 4;
                uint32_t defaultTarget = instr.address + (instr.ops[2] << 2);
				targets.push_back(defaultTarget);
            }
			else if (strcmp(instr.opcName.c_str(), "cmplwi") == 0 && field == instr.ops[0]) // check if the field is the same as the BC
			{
                numTargets = instr.ops[3] + 1;
                break;
			}
        }
    }
};
