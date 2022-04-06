#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "Value.h"

namespace lox
{
class Chunk
{
public:
    enum OpCode
    {
        kOpReturn,
        kOpConstant,
        kOpNegate,
        kOpAdd,
        kOpSubtract,
        kOpMultiply,
        kOpDivide
    }; // end OpCode

    Chunk() = default;
    ~Chunk() = default;

    Chunk(const Chunk&) = default;
    Chunk& operator=(const Chunk&) = default;

    Chunk(Chunk&&) = default;
    Chunk& operator=(Chunk&&) = default;

    const std::vector<uint8_t>& GetCode() const
        { return code_; }

    const std::vector<value::value_t>& GetConstants() const
        { return constants_; }

    void Write(uint8_t byte, int line);

    int AddConstant(value::value_t value);

    void Disassemble(const std::string& name) const;

    void Disassemble(int offset) const;
private:
    std::size_t DisassembleInstruction(int offset) const;

    std::size_t DisassembleSimpleInstruction(const std::string& name,
                                             int offset) const;

    std::size_t DisassembleConstantInstruction(const std::string& name,
                                               int offset) const;

    std::vector<uint8_t>        code_;
    std::vector<value::value_t> constants_;
    std::vector<int>            lines_;
}; // end Chunk
} // end lox
