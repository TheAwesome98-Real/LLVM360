#include "IRFunc.h"
#include <sstream>




//
// Code Blocks
//

void IRFunc::genBody()
{

}

llvm::BasicBlock* IRFunc::createBasicBlock(uint32_t address)
{
    std::ostringstream oss{};
    oss << "bb_" << std::hex << std::setfill('0') << std::setw(8) << address;
    return llvm::BasicBlock::Create(m_irGen->m_module->getContext(), oss.str(), m_irFunc);
}

llvm::BasicBlock* IRFunc::getCreateBBinMap(uint32_t address)
{
    if (isBBinMap(address)) {
        return codeBlocks.at(address)->bb_Block;
    }
    CodeBlock* block = new CodeBlock();
    block->bb_Block = createBasicBlock(address);
	block->address = address;
    codeBlocks.try_emplace(address, block);
    return block->bb_Block;
}

bool IRFunc::isBBinMap(uint32_t address)
{
    return codeBlocks.find(address) != codeBlocks.end();
}