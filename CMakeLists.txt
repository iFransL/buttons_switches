#
# Copyright (c) 2021 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(MyKeys)

# NORDIC SDK APP START
target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/keys.c)
target_sources(app PRIVATE src/keys.h)
# NORDIC SDK APP END

# Include source directory
#target_include_directories(app PRIVATE src)

# Application directories
#add_subdirectory(src/fonts)