cmake_minimum_required(VERSION 3.10)
project(KafkaLogProducer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(OpenCV_ROOT "${VCPKG_INSTALLED_DIR}/x64-linux/share/opencv4")
# include(${CMAKE_TOOLCHAIN_FILE})

# 查找 librdkafka++
find_package(PkgConfig REQUIRED)

find_package(RdKafka CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED) 
# find_package(Boost REQUIRED)
find_package(Boost CONFIG REQUIRED COMPONENTS filesystem system)

# 头文件路径
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/include
    ${Boost_INCLUDE_DIRS}
)

# 源文件
file(GLOB_RECURSE SRCS ${CMAKE_SOURCE_DIR}/src/*.hpp)
add_executable(kafka_producer ./src/main.cpp ${SRCS})

# 链接库
target_link_libraries(kafka_producer PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(kafka_producer PRIVATE RdKafka::rdkafka RdKafka::rdkafka++)
target_link_libraries(kafka_producer PRIVATE Boost::filesystem Boost::system)

# 如果你要确保静态链接
# set_target_properties(kafka_producer PROPERTIES LINK_SEARCH_START_STATIC ON)
# set_target_properties(kafka_producer PROPERTIES LINK_SEARCH_END_STATIC ON)

# 添加测试子目录
enable_testing()
add_subdirectory(tests)
