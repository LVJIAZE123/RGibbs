cmake_minimum_required(VERSION 3.15)
project(RGibbs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(CTest)

add_library(RGibbs
    src/RGibbsReactor.cpp
)

target_include_directories(RGibbs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

enable_testing()
if(BUILD_TESTING)
    add_executable(rgibbs_tests
        tests/test_reactor.cpp
    )
    target_link_libraries(rgibbs_tests PRIVATE RGibbs)
    add_test(NAME RGibbsTests COMMAND rgibbs_tests)
endif()
