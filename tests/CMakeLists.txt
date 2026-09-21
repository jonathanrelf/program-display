# SPDX-FileCopyrightText: 2026 Jonathan Relf and contributors
# SPDX-License-Identifier: GPL-2.0-or-later

cmake_minimum_required(VERSION 3.20)

project(program_display_domain_tests LANGUAGES CXX)

enable_testing()

add_library(program_display_domain
  ../src/domain/display_identity.cpp
)
target_include_directories(program_display_domain PUBLIC ../src/domain)
target_compile_features(program_display_domain PUBLIC cxx_std_20)

add_executable(display_identity_tests domain/display_identity_tests.cpp)
target_link_libraries(display_identity_tests PRIVATE program_display_domain)

add_test(NAME display_identity_tests COMMAND display_identity_tests)

add_executable(
  output_request_policy_tests
  app/output_request_policy_tests.cpp
  ../src/app/output_request_policy.cpp
)
target_include_directories(output_request_policy_tests PRIVATE ../src)
target_link_libraries(output_request_policy_tests PRIVATE program_display_domain)
target_compile_features(output_request_policy_tests PRIVATE cxx_std_20)
add_test(NAME output_request_policy_tests COMMAND output_request_policy_tests)
