# GoogleTest lookup order: installed package -> distro sources (/usr/src/googletest) -> download.
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    if(EXISTS "/usr/src/googletest/CMakeLists.txt")
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        add_subdirectory(/usr/src/googletest ${CMAKE_BINARY_DIR}/googletest EXCLUDE_FROM_ALL)
        add_library(GTest::gtest ALIAS gtest)
        add_library(GTest::gtest_main ALIAS gtest_main)
    else()
        include(FetchContent)
        FetchContent_Declare(googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
        FetchContent_MakeAvailable(googletest)
    endif()
endif()

include(GoogleTest)

add_executable(smartcounter_tests
    test_tracker.cpp
    test_line_counter.cpp
)
target_link_libraries(smartcounter_tests PRIVATE smartcounter_core GTest::gtest_main)
gtest_discover_tests(smartcounter_tests)
