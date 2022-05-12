# lox

Lox is a C-style, interpreted programming language. The lox language definition
and implementation follows directly from Robert Nystrom's
[Crafting Interpreters](https://craftinginterpreters.com/) text. Release v1.0
implements lox as a
[tree walk interpreter](https://craftinginterpreters.com/a-tree-walk-interpreter.htAml).
Release v2.0 implements lox using a [bytecode virtual machine](https://craftinginterpreters.com/a-bytecode-virtual-machine.html).
Both versions of lox were written in C++.

### Running the lox Interpreter

#### Local Build

To build and run lox directly on your PC, you  will need:

* A C++ compiler supporting C++17 features
* CMake3.13+

You can build the interpreter with the following command:

```
cd cpplox/scripts && ./build_lox.sh
```

The `lox` interpreter will be installed under `cpplox/bin/lox`.
You can run the `lox` executable directly to get a REPL or
you can pass the interpreter a lox script (i.e., `lox <SCRIPT_NAME>`).

#### Docker Image

If you rather not install the tools needed to build lox on your PC, you can
use [Docker](https://docs.docker.com/engine/install/) to experiment with
the interpreter.

To run the latest lox docker image:

```
docker run --rm -it iguerra130154/lox:latest /bin/sh
```
When the image launches, you will be placed in the `/examples` directory. You
can call the `lox` interpreter directly from within the container. Try it out
by running one of the example scripts:

```
lox func.lox
```

### Project Documentation

This project is documented using [Doxygen](https://www.doxygen.nl/index.html).
Project docs can be viewed in HTML. To build the project documentation, follow
the steps in [Local Build](#local-build) and replace the build command with:

```
cd cpplox/scripts && ./build_lox.sh -d
```

After running the above command, lox docs will be installed to the project
root directory under `docs/cpplox`.
