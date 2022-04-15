#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include "VirtualMachine.h"

static lox::VirtualMachine::InterpretResult Interpret(
    const std::string& source)
{
    lox::VirtualMachine vm;
    return vm.Interpret(source);
}

static void Repl()
{
    const std::string kPrompt = "lox >>> ";
    std::printf("%s", kPrompt.c_str());

    std::string line;
    while (std::getline(std::cin, line)) {
        Interpret(line);
        std::printf("%s", kPrompt.c_str());
    }
}

static void RunFile(const std::string& script)
{
    std::ifstream script_fd(script);
    if (!script_fd.is_open()) {
        std::fprintf(stderr, "error: unable to open script '%s'\n",
                     script.c_str());
        exit(74);
    }

    /* Read the User script in one fell swoop. */
    std::stringstream buffer;
    buffer << script_fd.rdbuf();

    using InterpretResult = lox::VirtualMachine::InterpretResult;
    InterpretResult result = Interpret(buffer.str());
    if (InterpretResult::kInterpretCompileError == result)
        exit(65);
    if (InterpretResult::kInterpretRuntimeError == result)
        exit(70);
}

int main(int argc, char** argv)
{
    if (1 == argc) {
        Repl();
    } else if (2 == argc) {
        RunFile(argv[1]);
    } else {
        std::fprintf(stderr, "usage: lox [script_path]\n");
        exit(64);
    }

    exit(EXIT_SUCCESS);
}
