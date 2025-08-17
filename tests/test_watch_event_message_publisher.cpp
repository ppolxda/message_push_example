#include <gtest/gtest.h>
#include <opencv2/core.hpp>
#include <memory>
#include <string>
#include <vector>
#include "watch/messages/watch_message.hpp"

// Mock Publisher
class MockPublisher : public BasePublisher
{
public:
    std::vector<std::pair<std::string, std::string>> published_msgs;
    bool publish(const std::string &topic, const std::string &message) override
    {
        published_msgs.emplace_back(topic, message);
        return true;
    }
    bool create_topic(const std::string &topic, const std::map<std::string, std::string> &options = {}) override
    {
        return true;
    }
};

TEST(WatchEventMessagePublisherTest, AllCallbacksPublish)
{
    auto mock = std::make_unique<MockPublisher>();
    auto *mockPtr = mock.get();
    WatchEventMessagePublisher publisher(std::move(mock));

    publisher.matchStartCallback();
    publisher.matchEndCallback();
    publisher.cameraStreamCallback(cv::Mat(), cv::Mat());
    publisher.ballPositionCallback(cv::Point2f(1, 2), cv::Point2f(3, 4));
    std::vector<cv::Point3f> pos = {{1, 2, 3}, {4, 5, 6}};
    publisher.predTrackBallPositionCallback(pos);
    publisher.realTrackBallPositionCallback(pos);
    publisher.shuttlecockPositionCallback(cv::Point3f(7, 8, 9));

    // 检查所有回调都调用了publish
    EXPECT_EQ(mockPtr->published_msgs.size(), 7);
    // 可进一步检查每条消息内容
}
