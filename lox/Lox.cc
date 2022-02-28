#include <iostream>
#include <cstdlib>

#include "Toy.h"

int main()
{
    std::cout << "Running toy program..." << std::endl;
    lox::Toy toy;
    toy.PrintHelloWorld();
    std::cout << "If you saw the text \"Hello, world.\" on your screen, "
              << "then cpplox is installed correctly."
              << std::endl;

    exit(EXIT_SUCCESS);
}
