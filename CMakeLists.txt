cmake_minimum_required(VERSION 3.20)

project(glog-linker-repro NONE)

# Allow the Ninja generators to output messages as they happen by assigning
# these jobs to the 'console' job pool
set(console_access "")
if(CMAKE_GENERATOR MATCHES "^Ninja")
  set(
      console_access
      USES_TERMINAL_CONFIGURE YES
      USES_TERMINAL_BUILD YES
      USES_TERMINAL_INSTALL YES
  )
endif()

# Ensure we have compilers
find_program(gcc gcc REQUIRED)
find_program(gplusplus g++ REQUIRED)
find_program(clang clang REQUIRED)
find_program(clangplusplus clang++ REQUIRED)

include(ExternalProject)

# **** glog gcc ****
ExternalProject_Add(
  glog-gcc
  SOURCE_DIR "${PROJECT_SOURCE_DIR}/glog"
  CMAKE_CACHE_ARGS
  "-DCMAKE_C_COMPILER:PATH=${gcc}"
  "-DCMAKE_CXX_COMPILER:PATH=${gplusplus}"
  -DCMAKE_BUILD_TYPE:STRING=Release
  -DBUILD_SHARED_LIBS:BOOL=OFF
  -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
  EXCLUDE_FROM_ALL YES
  ${console_access}
  CONFIGURE_HANDLED_BY_BUILD YES
)
ExternalProject_Get_Property(glog-gcc install_dir)
set(glog_gcc_install_dir "${install_dir}")

# **** glog clang ****
ExternalProject_Add(
  glog-clang
  SOURCE_DIR "${PROJECT_SOURCE_DIR}/glog"
  CMAKE_CACHE_ARGS
  "-DCMAKE_C_COMPILER:PATH=${clang}"
  "-DCMAKE_CXX_COMPILER:PATH=${clangplusplus}"
  -DCMAKE_BUILD_TYPE:STRING=Release
  -DBUILD_SHARED_LIBS:BOOL=OFF
  -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
  EXCLUDE_FROM_ALL YES
  ${console_access}
  CONFIGURE_HANDLED_BY_BUILD YES
)
ExternalProject_Get_Property(glog-clang install_dir)
set(glog_clang_install_dir "${install_dir}")

# *** repro clang release (fails) ***
ExternalProject_Add(
  repro-clang-rel
  SOURCE_DIR "${PROJECT_SOURCE_DIR}/repro"
  CMAKE_CACHE_ARGS
  "-DCMAKE_C_COMPILER:PATH=${clang}"
  "-DCMAKE_CXX_COMPILER:PATH=${clangplusplus}"
  -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
  -DCMAKE_BUILD_TYPE:STRING=Release
  "-Dglog_DIR:PATH=${glog_gcc_install_dir}/lib/cmake/glog"
  INSTALL_COMMAND ""
  EXCLUDE_FROM_ALL YES
  DEPENDS glog-gcc
  CONFIGURE_HANDLED_BY_BUILD YES
)

# *** repro clang debug (passes) ***
ExternalProject_Add(
  repro-clang-dbg
  SOURCE_DIR "${PROJECT_SOURCE_DIR}/repro"
  CMAKE_CACHE_ARGS
  "-DCMAKE_C_COMPILER:PATH=${clang}"
  "-DCMAKE_CXX_COMPILER:PATH=${clangplusplus}"
  -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
  -DCMAKE_BUILD_TYPE:STRING=Debug
  "-Dglog_DIR:PATH=${glog_gcc_install_dir}/lib/cmake/glog"
  INSTALL_COMMAND ""
  EXCLUDE_FROM_ALL YES
  DEPENDS glog-gcc
  CONFIGURE_HANDLED_BY_BUILD YES
)

# *** repro gcc release against clang glog (passes) ***
ExternalProject_Add(
  repro-gcc-rel
  SOURCE_DIR "${PROJECT_SOURCE_DIR}/repro"
  CMAKE_CACHE_ARGS
  "-DCMAKE_C_COMPILER:PATH=${gcc}"
  "-DCMAKE_CXX_COMPILER:PATH=${gplusplus}"
  -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
  -DCMAKE_BUILD_TYPE:STRING=Release
  "-Dglog_DIR:PATH=${glog_clang_install_dir}/lib/cmake/glog"
  INSTALL_COMMAND ""
  EXCLUDE_FROM_ALL YES
  DEPENDS glog-clang
  CONFIGURE_HANDLED_BY_BUILD YES
)

# *** repro gcc debug against clang glog (passes) ***
ExternalProject_Add(
  repro-gcc-dbg
  SOURCE_DIR "${PROJECT_SOURCE_DIR}/repro"
  CMAKE_CACHE_ARGS
  "-DCMAKE_C_COMPILER:PATH=${gcc}"
  "-DCMAKE_CXX_COMPILER:PATH=${gplusplus}"
  -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
  -DCMAKE_BUILD_TYPE:STRING=Debug
  "-Dglog_DIR:PATH=${glog_clang_install_dir}/lib/cmake/glog"
  INSTALL_COMMAND ""
  EXCLUDE_FROM_ALL YES
  DEPENDS glog-clang
  CONFIGURE_HANDLED_BY_BUILD YES
)
