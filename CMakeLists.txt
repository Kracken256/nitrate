cmake_minimum_required(VERSION 3.15)
project(quix-compiler-suite CXX)

option(MEMORY_OVER_SPEED "Optimize for memory over speed" OFF)
option(COVERAGE "Enable coverage" OFF)
option(STRIP_OUTPUT "Strip output" OFF)
option(BUILD_TESTING "Build tests" ON)

if (BUILD_TESTING)
  enable_testing()
endif()

# Weird compiler warning with macro arguments. Disable it.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=gnu-zero-variadic-macro-arguments -Wno-gnu-zero-variadic-macro-arguments")

# Modern security hardening flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector -fPIC")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,relro,-z,now")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -pedantic -Werror \
                    -Wno-error=unused-parameter \
                    -Wno-error=unused-command-line-argument \
                    -Wno-unused-command-line-argument \
                    -D__QUIX_IMPL__")

# Enable full optimization in general
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

if (STRIP_OUTPUT STREQUAL "ON")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s")
endif()

# Inject the optimization intent
if (MEMORY_OVER_SPEED)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMEMORY_OVER_SPEED=1")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMEMORY_OVER_SPEED=0")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  # Fix issue with ASAN and private aliases
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mllvm -asan-use-private-alias=1")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -fsanitize=undefined")
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG -fvisibility=hidden -fvisibility-inlines-hidden -flto")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -flto")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
else ()
  message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
endif()

if (COVERAGE STREQUAL "ON")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  #================== CREATE DEBUG BUILD ==================
  add_subdirectory(libquix-core)
  add_subdirectory(libquix-lexer)
  add_subdirectory(libquix-prep)
  add_subdirectory(libquix-parser)
  add_subdirectory(libquix-qxir)
  add_subdirectory(libquix-codegen)
  add_subdirectory(libquix)
  add_subdirectory(qpkg)
  # add_subdirectory(extensions/quixd)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  #================= CREATE RELEASE BUILD =================
  add_subdirectory(libquix-core)
  add_subdirectory(libquix-lexer)
  add_subdirectory(libquix-prep)
  add_subdirectory(libquix-parser)
  add_subdirectory(libquix-qxir)
  add_subdirectory(libquix-codegen)
  add_subdirectory(libquix)
  add_subdirectory(qpkg)
  # add_subdirectory(extensions/quixd)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  foreach(lib quix-core quix-core-shared 
              quix-lexer quix-lexer-shared 
              quix-prep quix-prep-shared
              quix-parser quix-parser-shared
              quix-qxir quix-qxir-shared
              quix-codegen quix-codegen-shared
              quix quix-shared)
    target_compile_options(${lib} PRIVATE -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined)
    # Check if it ends in '-shared'
    if (${lib} MATCHES ".*-shared$")
      target_link_options(${lib} PRIVATE -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined)
    endif()
  endforeach()
endif()

if (BUILD_TESTING)
  foreach(lib libquix-basic-lexer-fuzz libquix-prep-fuzzer libquix-qxir-fuzzer)
    target_compile_options(${lib} PRIVATE -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined)
    target_link_options(${lib} PRIVATE -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined)
  endforeach()
endif()
