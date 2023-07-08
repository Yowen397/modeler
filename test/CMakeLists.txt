
include_directories(../include)

add_executable(test_common test_common.cpp)
add_test(NAME test_common COMMAND test_common)