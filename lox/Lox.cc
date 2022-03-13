#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>

#include "Stmt.h"
#include "Parser.h"
#include "Scanner.h"
#include "Interpreter.h"
#include "ErrorLogger.h"

void Run(const std::string& source)
{
    lox::Scanner            scanner(source);
    lox::Parser             parser(scanner.ScanTokens());
    static lox::Interpreter interpreter;

    /* Parse program statements. */
    std::vector<std::shared_ptr<lox::ast::Stmt>> statements = parser.Parse();

    /* Interpret! */
    interpreter.Interpret(statements);
}

void RunPrompt()
{
    const std::string kPrompt = "lox >>> ";
    std::string line;
    lox::ErrorLogger& err_logger = lox::ErrorLogger::GetInstance();

    std::cout << kPrompt;
    while (std::getline(std::cin, line)) {
        Run(line);

        /* Don't end an interactive session over a single error. */
        if (err_logger.HadStaticError())
            err_logger.ClearStaticError();
        if (err_logger.HadRuntimeError())
            err_logger.ClearRuntimeError();

        std::cout << kPrompt;
    }
}

void RunFile(const std::string& script)
{
    std::ifstream script_fd(script);
    if (!script_fd.is_open()) {
        std::cerr << "error, unable to open script '"
                  << script << "'" << std::endl;
        return;
    }

    /* Read the User script in one fell swoop. */
    std::stringstream buffer;
    buffer << script_fd.rdbuf();
    Run(buffer.str());
}

int main(int argc, char** argv)
{
    if (argc > 2) {
        std::cerr << "usage: lox [script]" << std::endl;
        exit(EXIT_FAILURE);
    } else if (2 == argc) {
        /* Run user script. */
        RunFile(argv[1]);
    } else {
        /* Run interactively. */
        RunPrompt();
    }

    lox::ErrorLogger& err = lox::ErrorLogger::GetInstance();
    if (err.HadStaticError() || err.HadRuntimeError())
        exit(EXIT_FAILURE);

    exit(EXIT_SUCCESS);
}
