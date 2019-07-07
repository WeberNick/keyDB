CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

FIND_PROGRAM(CMAKE_CXX_COMPILER NAMES $ENV{CXX} g++ PATHS ENV PATH NO_DEFAULT_PATH)

PROJECT(keyDB LANGUAGES CXX VERSION 0.0.1)

#Use standard gcc and g++ in env path if not else specified
SET(CMAKE_CXX_COMPILER /usr/local/Cellar/gcc/8.2.0/bin/g++)

#Set c++ standard, warning flags and linker flags
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -march=native -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef -Werror")
SET(CMAKE_VERBOSE_MAKEFILE OFF)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin/${CMAKE_BUILD_TYPE})
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/build/${CMAKE_BUILD_TYPE})

FIND_PACKAGE(Boost 1.67 COMPONENTS system REQUIRED)
FIND_PACKAGE(OpenSSL REQUIRED)
FIND_PACKAGE(Threads)

ADD_SUBDIRECTORY(src)
ENABLE_TESTING()
ADD_SUBDIRECTORY(test)
