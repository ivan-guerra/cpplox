#include <cstdlib>

#include "Chunk.h"

int main(void)
{
    lox::Chunk chunk;
    int constant = chunk.AddConstant(1.2);
    chunk.Write(lox::Chunk::OpCode::kOpConstant, 123);
    chunk.Write(constant, 123);
    chunk.Write(lox::Chunk::OpCode::kOpReturn, 123);
    chunk.Disassemble("test chunk");

    exit(EXIT_SUCCESS);
}
