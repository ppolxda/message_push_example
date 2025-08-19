#pragma once
#include "./event/base_event.hpp"
#include "./event/hook_event_publisher.hpp"
#include "./publisher/factory_publisher.hpp"

namespace hook_event {

class HookEvent {
 public:
  HookEvent(
      std::string broker = "localhost:9092",
      std::string topic = "test",
      std::string topic_image = "test_image") {
    // 初始化消息中间件
    publisher::KafkaPublisherConfig config;
    config.bootstrapServers = broker;
    publisher_ = publisher::PublisherFactory::createKafkaPublisher(config);
    auto event = std::make_shared<event::HookEventPublisher>(publisher_);

    // 初始化消息管理对象
    manager_ = std::make_shared<event::EventManager>();
    manager_->addCallback(event);
    manager_->start();
    topic_ = topic;
    topic_image_ = topic_image;
  }
  ~HookEvent() {
    manager_.reset();
    publisher_.reset();
  }

  // 单例获取方法
  static HookEvent& getInstance(
      const std::string& broker = "localhost:9092",
      const std::string& topic = "test",
      const std::string& topic_image = "test_image") {
    static std::once_flag flag;
    static HookEvent* instance = nullptr;
    std::call_once(
        flag, [&]() { instance = new HookEvent(broker, topic, topic_image); });
    return *instance;
  }

 public:
  //   manager.emit(EnumEventType::MatchStart);
  //   manager.emit(EnumEventType::CameraStream, imga, imgb);
  //   manager.emit(EnumEventType::BallPosition, cv::Point2f(1, 2), cv::Point2f(3, 4));
  //   manager.emit(EnumEventType::MatchEnd);
  event::EventManager& getManager() {
    return *manager_;
  }

 private:
  // 禁止拷贝和赋值
  //   HookEvent(const HookEvent&) = delete;
  //   HookEvent& operator=(const HookEvent&) = delete;
  //   HookEvent(HookEvent&&) = delete;
  //   HookEvent& operator=(HookEvent&&) = delete;

 public:
  std::shared_ptr<event::EventManager> manager_;
  std::shared_ptr<publisher::BasePublisher> publisher_;
  std::string topic_;
  std::string topic_image_;
};

};  // namespace hook_event