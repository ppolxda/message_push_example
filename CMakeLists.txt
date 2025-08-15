cmake_minimum_required(VERSION 3.10)
project(KafkaLogProducer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 librdkafka++
find_package(PkgConfig REQUIRED)
find_package(RdKafka CONFIG REQUIRED)

find_package(librdkafka CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(eventpp CONFIG REQUIRED)

# 头文件路径
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${RDKAFKA_INCLUDE_DIRS}
)

# nlohmann/json（单头文件库，可以直接下载到 include/）
include_directories(${CMAKE_SOURCE_DIR}/include)

# 源文件
add_executable(kafka_producer main.cpp kafka_publish.hpp)

# 链接库
target_link_libraries(kafka_producer PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(kafka_producer PRIVATE RdKafka::rdkafka RdKafka::rdkafka++)
target_link_libraries(kafka_producer PRIVATE eventpp::eventpp)

# 如果你要确保静态链接
# set_target_properties(kafka_producer PROPERTIES LINK_SEARCH_START_STATIC ON)
# set_target_properties(kafka_producer PROPERTIES LINK_SEARCH_END_STATIC ON)
