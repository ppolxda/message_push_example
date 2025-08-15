
find_package(GTest CONFIG REQUIRED)
find_package(OpenCV REQUIRED)

add_executable(test_message ${CMAKE_SOURCE_DIR}/tests/test_message.cpp)
target_link_libraries(test_message 
    PRIVATE GTest::gtest_main 
    PRIVATE eventpp::eventpp 
    PRIVATE nlohmann_json::nlohmann_json 
    PRIVATE RdKafka::rdkafka 
    PRIVATE RdKafka::rdkafka++ 
    PRIVATE opencv_core 
    PRIVATE opencv_imgproc 
    PRIVATE opencv_highgui
)
add_test(NAME test_message COMMAND test_message)
