FROM ubuntu:22.04

RUN apt-get update && \
    apt-get install -y build-essential clang cmake libgmock-dev libgflags-dev ninja-build