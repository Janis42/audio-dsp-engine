# ==============================================================================
# tests/CMakeLists.txt — Google Test integration for audio-dsp-engine, written by Claude Opus
# ==============================================================================

# --- 1. Fetch GoogleTest -------------------------------------------------------
# FetchContent is a built-in CMake module that downloads dependencies at
# configure time (when you run `cmake ..`), so you don't have to install
# GoogleTest system-wide or vendor it into your repo.
include(FetchContent)

FetchContent_Declare(
    googletest                                              # logical name (arbitrary, used internally by CMake)
    GIT_REPOSITORY https://github.com/google/googletest.git # where to clone from
    GIT_TAG        v1.15.2                                  # pin to a specific release for reproducibility
)

# On Windows with MSVC, GoogleTest defaults to linking the C runtime statically
# (/MT). This flag forces it to use the shared CRT (/MD) so it matches our
# project.  Harmless on Linux/macOS, but good practice to always set it.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Actually download, build, and make GoogleTest targets available.
# After this line, targets like GTest::gtest and GTest::gtest_main exist.
FetchContent_MakeAvailable(googletest)

# --- 2. Create the test executable --------------------------------------------
# One executable per test suite keeps things modular.  As the project grows you
# can add more add_executable() blocks or glob additional test_*.cpp files.
add_executable(audio-dsp-engine-tests
    test_radix2DIT.cpp
)

# --- 3. Include paths ---------------------------------------------------------
# Tests need to #include project headers.  We expose both:
#   • src/           — so tests can reach implementation files like audio_processing/radix2DIT.cpp
#   • include/       — the public header directory (currently empty, but ready for future use)
# Using PROJECT_SOURCE_DIR keeps the path correct regardless of where the build
# directory is located.
target_include_directories(audio-dsp-engine-tests PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/include
)

# --- 4. Link GoogleTest -------------------------------------------------------
# GTest::gtest_main provides both the Google Test framework AND a default main()
# function that runs all TEST() macros — so your test .cpp files don't need
# their own main().
# If you ever need a custom main(), link GTest::gtest instead and write your own.
target_link_libraries(audio-dsp-engine-tests PRIVATE
    GTest::gtest_main
)

# --- 5. Register tests with CTest --------------------------------------------
# The GoogleTest CMake module ships with CMake itself.  gtest_discover_tests()
# runs the compiled test binary with --gtest_list_tests at build time and
# registers each TEST()/TEST_F() as an individual CTest test case.
# This means `ctest` output shows per-test pass/fail rather than one blob.
include(GoogleTest)
gtest_discover_tests(audio-dsp-engine-tests)
