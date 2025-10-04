cmake_minimum_required(VERSION 3.15)

project(Naive+)


set(Loader 
    src/Loader/PEImage.h
    src/Loader/XexImage.h
    src/Loader/ImageLoader.cpp
    src/Loader/ImageLoader.h
    src/Loader/table/ImportTable.h
    src/Loader/AES/AES.h
    src/Loader/AES/AES.cpp
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
)

set(SRC
    src/LLVM360.cpp
    src/Shared.h
    src/Logger.h 

    ${Loader}
    ${DECODER}
    ${IR}
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_library(Naive+ SHARED ${SRC})
target_link_libraries(Naive+ PRIVATE ext_llvm)
