#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>

#include "Scanner.h"
#include "ErrorLogger.h"

void Run(const std::string& source)
{
    lox::Scanner scanner(source);
    std::vector<lox::Token> tokens = scanner.ScanTokens();

    for (const lox::Token& token : tokens)
        std::cout << token << std::endl;
}

void RunPrompt()
{
    const std::string kPrompt = "lox >>> ";
    std::string line;
    lox::ErrorLogger& err_logger = lox::ErrorLogger::GetInstance();

    std::cout << kPrompt;
    while (std::getline(std::cin, line)) {
        Run(line);
        if (err_logger.HadError())
            /* Don't end an interactive session over a single error. */
            err_logger.ClearError();

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

    (lox::ErrorLogger::GetInstance().HadError() ? exit(EXIT_FAILURE) :
                                                  exit(EXIT_SUCCESS));
}
