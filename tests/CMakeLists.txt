
find_package(GTest CONFIG REQUIRED)


set(TEST_LIBS
    GTest::gtest_main
    eventpp::eventpp
    nlohmann_json::nlohmann_json
    RdKafka::rdkafka
    RdKafka::rdkafka++
    opencv_core
    opencv_imgproc
    opencv_highgui
    aklomp::base64
)

add_executable(test_event ${CMAKE_SOURCE_DIR}/tests/test_event.cpp ${SRCS})
target_link_libraries(test_event PRIVATE ${TEST_LIBS})


add_executable(test_publisher ${CMAKE_SOURCE_DIR}/tests/test_publisher.cpp ${SRCS})
target_link_libraries(test_publisher PRIVATE ${TEST_LIBS})


add_test(NAME test_event COMMAND test_event)
add_test(NAME test_publisher COMMAND test_publisher)
