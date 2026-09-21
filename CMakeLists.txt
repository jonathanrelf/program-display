# SPDX-FileCopyrightText: 2026 Jonathan Relf and contributors
# SPDX-License-Identifier: GPL-2.0-or-later

cmake_minimum_required(VERSION 3.28...3.30)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version} LANGUAGES C CXX)

if(NOT OS_MACOS)
  message(FATAL_ERROR "Program Display 0.1 supports macOS only.")
endif()
enable_language(OBJCXX)

option(ENABLE_FRONTEND_API "Use obs-frontend-api for UI functionality" ON)
option(ENABLE_QT "Use Qt functionality" ON)
option(ENABLE_DISPLAY_IDENTITY_PROBE "Log diagnostic-only macOS display identity evidence" OFF)

include(compilerconfig)
include(defaults)
include(helpers)

add_library(${CMAKE_PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
find_package(obs-frontend-api REQUIRED)
find_package(Qt6 6.11.1 EXACT REQUIRED COMPONENTS Core Gui Widgets)

target_link_libraries(
  ${CMAKE_PROJECT_NAME}
  PRIVATE OBS::libobs OBS::obs-frontend-api Qt6::Core Qt6::Gui Qt6::Widgets
)
target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE cxx_std_20)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
if(ENABLE_DISPLAY_IDENTITY_PROBE)
  target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE PROGRAM_DISPLAY_ENABLE_DISPLAY_IDENTITY_PROBE=1)
endif()
target_compile_options(
  ${CMAKE_PROJECT_NAME}
  PRIVATE $<$<CXX_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header -Wno-comma>
)

target_sources(
  ${CMAKE_PROJECT_NAME}
  PRIVATE
    src/plugin-main.cpp
    src/app/output_controller.cpp
    src/app/output_controller.hpp
    src/app/output_request_policy.cpp
    src/app/output_request_policy.hpp
    src/domain/display_identity.cpp
    src/domain/display_identity.hpp
    src/obs/obs_adapters.cpp
    src/obs/obs_adapters.hpp
    src/qt/program_display_dock.cpp
    src/qt/program_display_dock.hpp
    src/qt/display_metadata.cpp
    src/qt/display_metadata.hpp
)

target_sources(
  ${CMAKE_PROJECT_NAME}
  PRIVATE
    src/platform/macos/display_metadata.hpp
    src/platform/macos/display_metadata.mm
)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE "-framework AppKit" "-framework CoreGraphics")
if(ENABLE_DISPLAY_IDENTITY_PROBE)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE "-framework IOKit")
endif()

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})
