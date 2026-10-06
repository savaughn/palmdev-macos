# =============================================================================
# Palm OS 68K application with the modern gcc (C17 / C++17)
#
# Instructions:
#   1. Build the compiler once: ../m68k-gcc/build-toolchain.sh
#      (build-prc comes from ../setup-toolchain.sh)
#   2. Copy this Makefile, main.cpp and ../templates/app.rcp into a new
#      directory next to the templates (e.g. palmdev-macos/myapp/), or set
#      PALMDEV to the palmdev-macos directory
#   3. Adjust APP_NAME, CREATOR_ID, SOURCES and run "make"
#
# C and C++ files can be mixed. C++: no exceptions, no RTTI, no hosted
# standard library (the freestanding headers such as <array>, <optional>,
# <type_traits>, <cstdint> are available). PilotMain must be extern "C".
# =============================================================================

APP_NAME   = MyApp
CREATOR_ID = MyAp
SOURCES    = main.cpp
RESOURCES  = app.rcp
HEADERS    =

PALMDEV ?= $(abspath ..)
include $(PALMDEV)/m68k-gcc/palmos.mk
