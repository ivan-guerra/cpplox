#include <new>
#include <memory>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstdint>

#include "Value.h"
#include "Object.h"
#include "Chunk.h"

namespace lox
{
std::size_t
Chunk::DisassembleInstruction(int offset) const
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
        case OpCode::kOpNot:
            return DisassembleSimpleInstruction("OP_NOT", offset);
        case OpCode::kOpNil:
            return DisassembleSimpleInstruction("OP_NIL", offset);
         case OpCode::kOpTrue:
            return DisassembleSimpleInstruction("OP_TRUE", offset);
        case OpCode::kOpFalse:
            return DisassembleSimpleInstruction("OP_FALSE", offset);
        case OpCode::KOpEqual:
            return DisassembleSimpleInstruction("OP_EQUAL", offset);
         case OpCode::kOpGreater:
            return DisassembleSimpleInstruction("OP_GREATER", offset);
        case OpCode::kOpLess:
            return DisassembleSimpleInstruction("OP_LESS", offset);
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
        case OpCode::kOpPrint:
            return DisassembleSimpleInstruction("OP_PRINT", offset);
        case OpCode::kOpPop:
            return DisassembleSimpleInstruction("OP_POP", offset);
        case OpCode::kOpDefineGlobal:
            return DisassembleConstantInstruction("OP_DEFINE_GLOBAL", offset);
        case OpCode::kOpGetGlobal:
           return DisassembleConstantInstruction("OP_GET_GLOBAL", offset);
        case OpCode::kOpSetGlobal:
            return DisassembleConstantInstruction("OP_SET_GLOBAL", offset);
        case OpCode::kOpGetLocal:
            return DisassembleByteInstruction("OP_GET_LOCAL", offset);
        case OpCode::kOpSetLocal:
            return DisassembleByteInstruction("OP_SET_LOCAL", offset);
        case OpCode::kOpJumpIfFalse:
            return DisassembleJumpInstruction("OP_JUMP_IF_FALSE", 1, offset);
        case OpCode::kOpJump:
            return DisassembleJumpInstruction("OP_JUMP", 1, offset);
        case OpCode::kOpLoop:
            return DisassembleJumpInstruction("OP_LOOP", -1, offset);
        case OpCode::kOpCall:
            return DisassembleByteInstruction("OP_CALL", offset);
        case OpCode::kOpClosure: {
             offset++;
             uint8_t constant = code_[offset++];
             std::printf("%-16s %4d ", "OP_CLOSURE", constant);
             val::PrintValue(constants_[constant]);
             std::printf("\n");

             std::shared_ptr<obj::ObjFunction> function =
                obj::AsFunction(constants_[constant]);
             for (int j = 0; j < function->upvalue_count; ++j) {
                 int is_local = code_[offset++];
                 int index    = code_[offset++];
                 std::printf("%04d      |                     %s %d\n",
                    offset - 2, is_local ? "local" : "upvalue", index);
                }
             return offset;
        }
        case OpCode::kOpGetUpvalue:
            return DisassembleByteInstruction("OP_GET_UPVALUE", offset);
        case OpCode::kOpSetUpvalue:
            return DisassembleByteInstruction("OP_SET_UPVALUE", offset);
        case OpCode::kOpCloseUpvalue:
            return DisassembleSimpleInstruction("OP_CLOSE_UPVALUE", offset);
        case OpCode::kOpClass:
            return DisassembleConstantInstruction("OP_CLASS", offset);
        case OpCode::kOpSetProperty:
            return DisassembleConstantInstruction("OP_SET_PROPERTY", offset);
        case OpCode::kOpGetProperty:
            return DisassembleConstantInstruction("OP_GET_PROPERTY", offset);
        case OpCode::kOpMethod:
            return DisassembleConstantInstruction("OP_METHOD", offset);
        case OpCode::kOpInvoke:
            return DisassembleInvokeInstruction("OP_INVOKE", offset);
        case OpCode::kOpInherit:
            return DisassembleSimpleInstruction("OP_INHERIT", offset);
        case OpCode::kOpGetSuper:
            return DisassembleConstantInstruction("OP_GET_SUPER", offset);
        case OpCode::kOpSuperInvoke:
            return DisassembleInvokeInstruction("OP_SUPER_INVOKE", offset);
        default:
            std::fprintf(stderr, "unknown opcode %d\n", instruction);
            return (offset + 1);
    }
}

std::size_t
Chunk::DisassembleSimpleInstruction(const std::string& name, int offset) const
{
    std::printf("%s\n", name.c_str());
    return (offset + 1);
}

std::size_t
Chunk::DisassembleConstantInstruction(const std::string& name,
                                      int offset) const
{
    uint8_t constant = code_[offset + 1];
    std::printf("%-16s %4d '", name.c_str(), constant);
    val::PrintValue(constants_[constant]);
    std::printf("'\n");

    /* Note we add 2 not 1 on return because the constant instruction is
       2 bytes long. */
    return (offset + 2);
}

std::size_t
Chunk::DisassembleByteInstruction(const std::string& name, int offset) const
{
    uint8_t slot = code_[offset + 1];
    std::printf("%-16s %4d\n", name.c_str(), slot);
    /* We add 2 not 1 on return because byte instructions are 2 bytes long. */
    return (offset + 2);
}

std::size_t
Chunk::DisassembleJumpInstruction(const std::string& name,
                                  int sign,
                                  int offset) const
{
    uint16_t jump = static_cast<uint16_t>(code_[offset + 1] << 8);
    jump |= code_[offset + 2];
    std::printf("%-16s %4d -> %d\n", name.c_str(), offset,
                offset + 3 + sign * jump);
    return offset + 3;
}

std::size_t
Chunk::DisassembleInvokeInstruction(const std::string& name, int offset) const
{
    uint8_t constant  = code_[offset + 1];
    uint8_t arg_count = code_[offset + 2];

    std::printf("%-16s (%d args) %4d '", name.c_str(), arg_count, constant);
    val::PrintValue(constants_[constant]);
    std::printf("'\n");
    return offset + 3;
}

void
Chunk::Write(uint8_t byte, int line)
{
    try {
        code_.push_back(byte);
        lines_.push_back(line);
    } catch (const std::bad_alloc& e) {
        std::fprintf(stderr, "error: %s\n", e.what());
        exit(EXIT_FAILURE);
    }
}

int
Chunk::AddConstant(const val::Value& value)
{
    try {
        constants_.push_back(value);
        return (constants_.size() - 1);
    } catch (const std::bad_alloc& e) {
        std::fprintf(stderr, "error: %s\n", e.what());
        exit(EXIT_FAILURE);
    }
}

void
Chunk::Disassemble(const std::string& name) const
{
    std::printf("== %s ==\n", name.c_str());
    for (std::size_t offset = 0; offset < code_.size();)
        offset = DisassembleInstruction(offset);
    std::printf("\n");
}
} // end lox
