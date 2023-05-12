# Glog linker issue

This repo is to reproduce a linker issue when compiling [glog](https://github.com/google/glog) with default-installed GCC 11 and try to link it against an executable compile with default-installed Clang 14 on Ubuntu 22.04.

## Reproduce

To reproduce the issue, we only need a very simple source file [`main.cpp`](./repro/main.cpp) (replicated below).

```cpp
#define GOOGLE_STRIP_LOG 1

#include <glog/logging.h>

int main(int argc, char* argv[]) {
    // Initialize Google’s logging library.
    google::InitGoogleLogging(argv[0]);

    VLOG(2) << "stuffs";
}
```

If we remove `#define GOOGLE_STRIP_LOG 1` it will compile, but that doesn't seem like a great solution.

### Environment

Build the Docker image (or find yourself an Ubuntu 22.04 machine and install the necessary packages installed by the Dockerfile)

```sh
docker build -t repro .
```

Then, enter the Docker environment to execute the build steps

```sh
docker run --rm -ti -v $(pwd):/workspace -w /workspace repro /bin/bash
```

Some explanation on the options

* `docker run` will run a container with specified options
* `--rm` will remove the container after exiting
* `-ti` will enable interactive tty
* `-v $(pwd):/workspace` will mount the current directory to `/workspace` path in the container
* `-w /workspace` will set the default directory when entering the container
* `repro` the name of the Docker image that we build with `docker build`
* `/bin/bash` the command to first enter the container---a shell to execute further commands

### Build

We are using a CMake super build to control building both projects, which makes for fewer commands to execute. The general steps executed by the superbuild are as follows:

1. Build glog as a static library with GCC and install it [`CMakeLists.txt`](./CMakeLists.txt)
2. Build our `repro` project with Clang and link against glog we installed above

First, we will configure the superbuild

```sh
cmake --preset default
```

We have a few different build targets, and you can get a list like so:

```sh
cmake --build --list-presets
```

We are interested in `repro-clang-rel` and `repro-clang-dbg`.

`repro-clang-rel` will build glog with GCC and the [repro](./repro) project with Clang using CMake's `Release` build type. It will fail with a linker error:

```sh
$ cmake --build --preset repro-clang-rel
[...]
[14/16] Performing build step for 'repro-clang-rel'
FAILED: repro-clang-rel-prefix/src/repro-clang-rel-stamp/repro-clang-rel-build /workspace/build/repro-clang-rel-prefix/src/repro-clang-rel-stamp/repro-clang-rel-build
cd /workspace/build/repro-clang-rel-prefix/src/repro-clang-rel-build && /usr/bin/cmake --build . && /usr/bin/cmake -E touch /workspace/build/repro-clang-rel-prefix/src/repro-clang-rel-stamp/repro-clang-rel-build
[1/2] /usr/bin/clang++ -DGFLAGS_IS_A_DLL=0 -isystem /workspace/build/glog-gcc-prefix/include -O3 -DNDEBUG -MD -MT CMakeFiles/issue.dir/main.cpp.o -MF CMakeFiles/issue.dir/main.cpp.o.d -o CMakeFiles/issue.dir/main.cpp.o -c /workspace/repro/main.cpp
[2/2] : && /usr/bin/clang++ -O3 -DNDEBUG  CMakeFiles/issue.dir/main.cpp.o -o issue  /workspace/build/glog-gcc-prefix/lib/libglog.a  /usr/lib/x86_64-linux-gnu/libgflags.so.2.2.2  -lpthread && :
FAILED: issue
: && /usr/bin/clang++ -O3 -DNDEBUG  CMakeFiles/issue.dir/main.cpp.o -o issue  /workspace/build/glog-gcc-prefix/lib/libglog.a  /usr/lib/x86_64-linux-gnu/libgflags.so.2.2.2  -lpthread && :
/usr/bin/ld: /workspace/build/glog-gcc-prefix/lib/libglog.a(logging.cc.o): relocation R_X86_64_PC32 against undefined hidden symbol `_ZTCN6google10LogMessage9LogStreamE0_So' can not be used when making a PIE object
/usr/bin/ld: final link failed: bad value
clang: error: linker command failed with exit code 1 (use -v to see invocation)
ninja: build stopped: subcommand failed.
ninja: build stopped: subcommand failed.
```

demangling with `c++filt`:

```sh
$ c++filt _ZTCN6google10LogMessage9LogStreamE0_So
construction vtable for std::basic_ostream<char, std::char_traits<char> >-in-google::LogMessage::LogStream
```

However, when building with CMake's `Debug` build type, it will build successfully:

```sh
$ cmake --build --preset repro-clang-dbg
[...]
[6/8] Performing build step for 'repro-clang-dbg'
[1/2] /usr/bin/clang++ -DGFLAGS_IS_A_DLL=0 -isystem /workspace/build/glog-gcc-prefix/include -g -MD -MT CMakeFiles/issue.dir/main.cpp.o -MF CMakeFiles/issue.dir/main.cpp.o.d -o CMakeFiles/issue.dir/main.cpp.o -c /workspace/repro/main.cpp
[2/2] : && /usr/bin/clang++ -g  CMakeFiles/issue.dir/main.cpp.o -o issue  /workspace/build/glog-gcc-prefix/lib/libglog.a  /usr/lib/x86_64-linux-gnu/libgflags.so.2.2.2  -lpthread && :
[8/8] Completed 'repro-clang-dbg'
```

### Other experiments

There are two other build presets that compile glog with Clang and the repro project with GCC. These both pass: `repro-gcc-rel` and `repro-gcc-dbg`.

## Thoughts

It seems that optimizations play a part in whether the error appears.

Is compiling with different compiler vendors so unsupported that it would fail at different optimization levels?
