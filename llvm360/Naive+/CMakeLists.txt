cmake_minimum_required(VERSION 3.15)

project(Naive+)


set(XEX 
    src/Xex/XexLoader.h
    src/Xex/XexLoader.cpp
    src/Xex/XexImage.h
    src/Xex/ImportTable.h
    src/Xex/AES.h
    src/Xex/AES.cpp
)

set(MISC
    src/misc/Utils.h
    src/misc/Utils.cpp
)

set(DECODER
    src/Decoder/Instruction.h
    src/Decoder/InstructionDecoder.cpp
    src/Decoder/InstructionDecoder.h
)

set(IR
    src/IR/Unit/UnitTesting.h
    src/IR/InstructionEmitter.h
    src/IR/IRFunc.cpp
    src/IR/IRFunc.h
    src/IR/IRGenerator.cpp
    src/IR/IRGenerator.h
    src/IR/JumpTables.h
)

set(SRC
    src/LLVM360.cpp
    src/Util.h

    ${XEX}
    ${MISC}
    ${DECODER}
    ${IR}
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_executable(Naive+ ${SRC})
target_link_libraries(Naive+ PRIVATE ext_llvm)
