#include <cstdlib>

#include "Chunk.h"
#include "VirtualMachine.h"

int main(void)
{
    using OpCode = lox::Chunk::OpCode;

    lox::VirtualMachine vm;


    lox::Chunk chunk;
    int line_number = 123;
    int constant = chunk.AddConstant(1.2);
    chunk.Write(OpCode::kOpConstant, line_number);
    chunk.Write(constant, line_number);

    constant = chunk.AddConstant(3.4);
    chunk.Write(OpCode::kOpConstant, line_number);
    chunk.Write(constant, line_number);

    chunk.Write(OpCode::kOpAdd, line_number);

    constant = chunk.AddConstant(5.6);
    chunk.Write(OpCode::kOpConstant, line_number);
    chunk.Write(constant, line_number);

    chunk.Write(OpCode::kOpDivide, line_number);

    chunk.Write(OpCode::kOpNegate, line_number);
    chunk.Write(OpCode::kOpReturn, line_number);

    chunk.Disassemble("Test Chunk");

    vm.Interpret(chunk);

    exit(EXIT_SUCCESS);
}
