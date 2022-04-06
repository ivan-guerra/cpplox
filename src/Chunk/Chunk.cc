#include <new>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstdint>

#include "Value.h"
#include "Chunk.h"

namespace lox
{
std::size_t Chunk::DisassembleInstruction(int offset) const
{
    std::printf("%04d ", offset);
    if ((offset > 0) && (lines_[offset] == lines_[offset - 1]))
        std::printf("   | ");
    else
        std::printf("%4d ", lines_[offset]);

    uint8_t instruction = code_[offset];
    switch (instruction) {
        case OpCode::kOpReturn:
            return DisassembleSimpleInstruction("OP_RETURN", offset);
        case OpCode::kOpConstant:
            return DisassembleConstantInstruction("OP_CONSTANT", offset);
        case OpCode::kOpNegate:
            return DisassembleSimpleInstruction("OP_NEGATE", offset);
        case OpCode::kOpAdd:
            return DisassembleSimpleInstruction("OP_ADD", offset);
        case OpCode::kOpSubtract:
            return DisassembleSimpleInstruction("OP_SUBTRACT", offset);
        case OpCode::kOpMultiply:
            return DisassembleSimpleInstruction("OP_MULTIPLY", offset);
        case OpCode::kOpDivide:
            return DisassembleSimpleInstruction("OP_DIVIDE", offset);
        default:
            std::printf("Unknown opcode %d\n", instruction);
            return (offset + 1);
    }
}

std::size_t Chunk::DisassembleSimpleInstruction(const std::string& name,
                                                int offset) const
{
    std::printf("%s\n", name.c_str());
    return (offset + 1);
}

std::size_t Chunk::DisassembleConstantInstruction(const std::string& name,
                                                  int offset) const
{
    uint8_t constant = code_[offset + 1];
    std::printf("%-16s %4d '", name.c_str(), constant);
    value::PrintValue(constants_[constant]);
    std::printf("'\n");

    return (offset + 2);
}

void Chunk::Write(uint8_t byte, int line)
{
    try {
        code_.push_back(byte);
        lines_.push_back(line);
    } catch (const std::bad_alloc& e) {
        std::printf("error: %s\n", e.what());
        exit(EXIT_FAILURE);
    }
}

int Chunk::AddConstant(value::value_t value)
{
    try {
        constants_.push_back(value);
        return (constants_.size() - 1);
    } catch (const std::bad_alloc& e) {
        std::fprintf(stderr, "error: %s\n", e.what());
        exit(EXIT_FAILURE);
    }
}

void Chunk::Disassemble(const std::string& name) const
{
    std::printf("== %s ==\n", name.c_str());

    for (std::size_t offset = 0; offset < code_.size();)
        offset = DisassembleInstruction(offset);
}

void Chunk::Disassemble(int offset) const
{
    if (offset < 0 || offset > static_cast<int>(code_.size()))
        /* NOOP on invalid offsets. */
        return;

    DisassembleInstruction(offset);
}
} // end lox