# 所需 CMake 的最低版本
cmake_minimum_required(VERSION 3.0.0)

aux_source_directory(. SRC_LIST)

include_directories(../include)

# 设置项目名称
# project(modeler)

# 添加可执行文件
add_executable(modeler ${SRC_LIST})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 为目标（XX）设置属性（输出路径）
# PROPERTIES：后接目标对应属性
# RUNTIME_OUTPUT_DIRECTORY：属性，表示源文件编译后输出的文件存放的目录
# CMAKE_BINARY_DIR/bin：上面属性的值，这里表示构建目录（build目录，CMake 自动生成的目录）下的 bin 文件夹
# set_target_properties(test PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY CMAKE_BINARY_DIR/bin
# )
