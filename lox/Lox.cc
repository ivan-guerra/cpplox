#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include "VirtualMachine.h"

/*!
 * \enum LoxExitCode
 * \brief The LoxExitCode enum defines the possible interpreter exit codes.
 */
enum LoxExitCode
{
    kSuccess           = 0,  /*!< Indicates the interpreter exited gracefully. */
    kInvalidUsage      = 64, /*!< Indicates the interpreter was called with invalid arguments. */
    kInvalidScriptPath = 74, /*!< Indicates a nonexistent/invalid script path was specified by the User. */
    kCompileError      = 65, /*!< Indicates a compile time error. */
    kRuntimeError      = 70  /*!< Indicates a runtime error. */
};

static lox::vm::VirtualMachine::InterpretResult
Interpret(const std::string& source)
{
    /* The VM is a static object meaning its state persists throughout the life
       of the interpreter program. The latter is intentional and useful
       especially in the case of the REPL where we interpret lines of source
       code one at a time (i.e., call Interpret() repeatedly with the
       expectation the VM 'remembers' the code last executed). */
    static lox::vm::VirtualMachine vm;
    return vm.Interpret(source);
}

static void
Repl()
{
    const std::string kPrompt = "lox >>> ";
    std::printf("%s", kPrompt.c_str());

    std::string line;
    while (std::getline(std::cin, line)) {
        Interpret(line);
        std::printf("%s", kPrompt.c_str());
    }
}

static void
RunFile(const std::string& script)
{
    std::ifstream script_fd(script);
    if (!script_fd.is_open()) {
        std::fprintf(stderr, "error: unable to open script '%s'\n",
                     script.c_str());
        exit(LoxExitCode::kInvalidScriptPath);
    }

    /* Read the User script in one fell swoop. */
    std::stringstream buffer;
    buffer << script_fd.rdbuf();

    using InterpretResult = lox::vm::VirtualMachine::InterpretResult;
    InterpretResult result = Interpret(buffer.str());
    if (InterpretResult::kInterpretCompileError == result)
        exit(LoxExitCode::kCompileError);
    if (InterpretResult::kInterpretRuntimeError == result)
        exit(LoxExitCode::kRuntimeError);
}

int main(int argc, char** argv)
{
    if (1 == argc) {
        Repl();
    } else if (2 == argc) {
        RunFile(argv[1]);
    } else {
        std::fprintf(stderr, "usage: lox [script_path]\n");
        exit(LoxExitCode::kInvalidUsage);
    }
    exit(LoxExitCode::kSuccess);
}
