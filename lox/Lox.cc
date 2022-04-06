#include <cstdlib>

#include "Chunk.h"
#include "VirtualMachine.h"

int main(void)
{
    lox::VirtualMachine vm;

    lox::Chunk chunk;
    int constant = chunk.AddConstant(1.2);
    chunk.Write(lox::Chunk::OpCode::kOpConstant, 123);
    chunk.Write(constant, 123);

    constant = chunk.AddConstant(3.4);
    chunk.Write(lox::Chunk::OpCode::kOpConstant, 123);
    chunk.Write(constant, 123);

    chunk.Write(lox::Chunk::OpCode::kOpAdd, 123);

    constant = chunk.AddConstant(5.6);
    chunk.Write(lox::Chunk::OpCode::kOpConstant, 123);
    chunk.Write(constant, 123);

    chunk.Write(lox::Chunk::OpCode::kOpDivide, 123);

    chunk.Write(lox::Chunk::OpCode::kOpNegate, 123);
    chunk.Write(lox::Chunk::OpCode::kOpReturn, 123);
    chunk.Disassemble("test chunk");

    vm.Interpret(chunk);

    exit(EXIT_SUCCESS);
}
