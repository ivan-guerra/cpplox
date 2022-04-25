#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "Value.h"

namespace lox
{

/*!
 * \class Chunk
 * \brief The Chunk class represents a grouping of bytecode instructions.
 */
class Chunk
{
public:
    /*!
     * \enum OpCode
     * \brief The OpCode enum defines the instruction types supported by Lox.
     */
    enum OpCode
    {
        kOpReturn,
        kOpConstant,
        kOpNil,
        kOpTrue,
        kOpFalse,
        KOpEqual,
        kOpGreater,
        kOpLess,
        kOpNot,
        kOpNegate,
        kOpAdd,
        kOpSubtract,
        kOpMultiply,
        kOpDivide,
        kOpPrint,
        kOpPop,
        kOpDefineGlobal,
        kOpGetGlobal,
        kOpSetGlobal
    }; // end OpCode

    /* The defaults for compiler generated methods are appropriate. */
    Chunk() = default;
    ~Chunk() = default;
    Chunk(const Chunk&) = default;
    Chunk& operator=(const Chunk&) = default;
    Chunk(Chunk&&) = default;
    Chunk& operator=(Chunk&&) = default;

    /*!
     * \brief Return a read only view of the Chunk's bytecode.
     */
    const std::vector<uint8_t>& GetCode() const
        { return code_; }

    /*!
     * \brief Return a read only view of the Chunk's constants.
     */
    const std::vector<val::Value>& GetConstants() const
        { return constants_; }

    /*!
     * \brief Return a read only view of the Chunk's line array.
     */
    const std::vector<int>& GetLines() const
        { return lines_; }

    /*!
     * \brief Write a raw byte to the Chunk.
     *
     * \param byte A byte representing an opcode or some other piece of
     *             information that needs to be added to the Chunk.
     * \param line The line number in the source text associated with this
     *             bytecode.
     */
    void Write(uint8_t byte, int line);

    /*!
     * \brief Add a new Lox constant value to the Chunk.
     *
     * \param value A constant value parsed from the source text.
     *
     * \return The index of \a value in the Chunk's underlying constants
     *         array. This index is valuable when constructing a
     *         OpCode::kOpConstant instruction which has the form:
     *         OP_CONSTANT \<CONSTANT_INDEX\>
     */
    int AddConstant(const val::Value& value);

    /*!
     * \brief Disassemble all instructions in this Chunk.
     *
     * \param name A label that is printed to STDOUT prior the printing of
     *             decoded instructions to STDOUT.
     */
    void Disassemble(const std::string& name) const;

    /*!
     * \brief Disassemble an instruction at offset \a offset within this Chunk.
     *
     * If \a offset is out of bounds, Disassemble() will immediately return.
     */
    void Disassemble(int offset) const;

private:
    /*!
     * \brief Disassemble() helper function.
     *
     * Print the decoding of the instruction at offset \a offset within this
     * Chunk to STDOUT.
     *
     * \param offset Offset of the instruction being disassembled in #code_.
     *
     * \return The offset of the next instruction in the Chunk.
     */
    std::size_t DisassembleInstruction(int offset) const;

    /*!
     * \brief Print an opcode only instruction to STDOUT.
     */
    std::size_t DisassembleSimpleInstruction(const std::string& name,
                                             int offset) const;
    /*!
     * \brief Print a constant instruction to STDOUT.
     */
    std::size_t DisassembleConstantInstruction(const std::string& name,
                                               int offset) const;

    std::vector<uint8_t>    code_;      /*!< Vector of compiled bytecode instructions. */
    std::vector<val::Value> constants_; /*!< Vector of constants parsed from the source text. */
    std::vector<int>        lines_;     /*!< Vector of line numbers. Usage TBD. */
}; // end Chunk
} // end lox
