# Create a cpplox image. This Dockerfile will build an Alpine Linux image
# which includes a build of the lox interpreter. You can launch a cpplox
# container and immediately run lox in interactive mode or you can mount
# a directory with your custom lox scripts and run them through the interpeter
# (i.e., lox <LOX_SCRIPT>).

# This Dockerfile uses a multi-stage build. The first stage, builder,  builds
# the lox interpreter. The second stage copies the lox binary from builder
# to the final image.

FROM alpine:latest AS builder

# Install all the packages needed to build cpplox.
RUN apk add --no-cache \
        build-base     \
        cmake          \
        bash

# Copy the cpplox source tree.
COPY ./ /cpplox/

# Build cpplox using the build script included in the cpplox source tree.
WORKDIR /cpplox/scripts
RUN chmod +x build_lox.sh && \
    ./build_lox.sh

FROM alpine:latest

# Install all packages needed to run the lox interpreter. Included are the
# vim/nano text editors for editing lox scripts from within the container
# instance.
RUN apk add --no-cache \
        vim            \
        nano           \
        bash           \
        libgcc         \
        libstdc++

# Copy the lox binary from the builder image to this final image.
COPY --from=builder /cpplox/bin/lox /usr/local/bin

# Copy over a handful of example lox scripts.
COPY examples/ /examples/

WORKDIR /examples
