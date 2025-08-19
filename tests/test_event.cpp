#include <gtest/gtest.h>
#include <limits.h>
#include <unistd.h>

#include <atomic>
#include <memory>
#include <string>

#include "hook_event/event/base_event.hpp"
#include "hook_event/event/hook_event.hpp"
#include "hook_event/publisher/factory_publisher.hpp"

std::string getCurrentDir() {
  char buf[PATH_MAX];
  if (getcwd(buf, sizeof(buf))) {
    return std::string(buf);
  }
  return {};
}

class TestEventMessage : public EventMessage {
 public:
  std::atomic<bool> matchStartCalled{false};
  std::atomic<bool> matchEndCalled{false};
  void matchStartCallback() override {
    matchStartCalled = true;
  }
  void matchEndCallback() override {
    matchEndCalled = true;
  }
};

TEST(EventManagerTest, MatchStartAndEndCallbacks) {
  auto msg = std::make_shared<TestEventMessage>();
  EventManager manager;
  manager.addCallback(msg);
  manager.start();
  manager.emit(EnumEventType::MatchStart);

  while (manager.poll() != 0)
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

  EXPECT_TRUE(msg->matchStartCalled);
  manager.emit(EnumEventType::MatchEnd);

  while (manager.poll() != 0)
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

  EXPECT_TRUE(msg->matchEndCalled);
  manager.stop();
}

// Mock Publisher
class MockPublisher : public BasePublisher {
 public:
  std::vector<std::pair<std::string, std::string>> published_msgs;
  bool publish(
      const std::string &topic,
      const unsigned char *message,
      const size_t size) override {
    published_msgs.emplace_back(
        topic, std::string(reinterpret_cast<const char *>(message), size));
    return true;
  }
  bool create_topic(
      const std::string &topic,
      const std::map<std::string, std::string> &options = {}) override {
    return true;
  }
};

TEST(HookEventPublisherTest, AllCallbacksPublishMock) {
  auto mock = std::make_shared<MockPublisher>();
  auto *mockPtr = mock.get();
  HookEventPublisher event(mock);
  std::string pwd = getCurrentDir();
  cv::Mat imga = cv::imread(pwd + "/../../tests/data/00000.png");
  EXPECT_TRUE(!imga.empty() and imga.cols > 0 and imga.rows > 0);
  cv::Mat imgb = cv::imread(pwd + "/../../tests/data/00001.png");
  EXPECT_TRUE(!imgb.empty() and imgb.cols > 0 and imgb.rows > 0);

  event.matchStartCallback();
  event.matchEndCallback();
  event.cameraStreamCallback(imga, imgb);
  event.ballPositionCallback(cv::Point2f(1, 2), cv::Point2f(3, 4));
  // std::vector<cv::Point3f> pos = {{1, 2, 3}, {4, 5, 6}};
  // event.predTrackBallPositionCallback(pos);
  // event.realTrackBallPositionCallback(pos);
  // event.shuttlecockPositionCallback(cv::Point3f(7, 8, 9));

  // 检查所有回调都调用了publish
  EXPECT_EQ(mockPtr->published_msgs.size(), 5);
}

TEST(HookEventPublisherTest, KafkaEventFullTest) {
  // 创建消息中间件
  KafkaPublisherConfig config;
  config.bootstrapServers = "localhost:9092";
  auto publisher = PublisherFactory::createKafkaPublisher(config);

  // 创建事件回调
  auto event = std::make_shared<HookEventPublisher>(publisher);

  // 事件管理层
  EventManager manager;
  manager.addCallback(event);
  manager.start();

  std::string pwd = getCurrentDir();
  cv::Mat imga = cv::imread(pwd + "/../../tests/data/00000.png");
  EXPECT_TRUE(!imga.empty() and imga.cols > 0 and imga.rows > 0);
  cv::Mat imgb = cv::imread(pwd + "/../../tests/data/00001.png");
  EXPECT_TRUE(!imgb.empty() and imgb.cols > 0 and imgb.rows > 0);

  manager.emit(EnumEventType::MatchStart);
  manager.emit(EnumEventType::CameraStream, imga, imgb);
  manager.emit(EnumEventType::BallPosition, cv::Point2f(1, 2), cv::Point2f(3, 4));
  manager.emit(EnumEventType::MatchEnd);

  // std::vector<cv::Point3f> pos = {{1, 2, 3}, {4, 5, 6}};
  // event.predTrackBallPositionCallback(pos);
  // event.realTrackBallPositionCallback(pos);
  // event.shuttlecockPositionCallback(cv::Point3f(7, 8, 9));

  while (manager.poll() != 0)
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

  manager.stop();
}
