#include <iostream>
#include <cstdlib>

int main(int argc, char** argv)
{
    if (argc > 2) {
        std::cerr << "usage: lox [script]" << std::endl;
        exit(EXIT_FAILURE);
    } else if (2 == argc) {
        /* Run user script. */
        std::cout << "User script path: " << argv[1] << std::endl;
    } else {
        /* Run interactively. */
    }
    exit(EXIT_SUCCESS);
}
