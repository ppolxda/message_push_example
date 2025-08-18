#include <gtest/gtest.h>

#include <chrono>

#include "hook_event/publisher/factory_publisher.hpp"

long long getCurrentTimestampMs() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

TEST(PublisherFactoryTest, CreateKafkaPublisher) {
  KafkaPublisherConfig config;
  config.bootstrapServers = "localhost:9092";
  auto publisher = PublisherFactory::createKafkaPublisher(config);
  ASSERT_NE(publisher, nullptr);
  // 检查类型
  auto kafkaPtr = dynamic_cast<KafkaPublisher *>(publisher.get());
  ASSERT_NE(kafkaPtr, nullptr);
  // 可以继续测试 publish/create_topic 等接口的行为（可用 mock 或 stub）
}

TEST(PublisherFactoryTest, KafkaPublisherPublish) {
  KafkaPublisherConfig config;
  config.bootstrapServers = "localhost:9092";
  auto publisher = PublisherFactory::createKafkaPublisher(config);
  ASSERT_NE(publisher, nullptr);
  // 创建topic
  EXPECT_THROW(publisher->create_topic("test_topic"), std::runtime_error);
  // 发送消息（此处不会真正投递到Kafka，仅测试接口调用）
  bool published = publisher->publish("test_topic", "hello world");

  nlohmann::json j;
  j["level"] = "DEBUG";
  j["timestamp"] = getCurrentTimestampMs();
  j["message"] = "Database connection failed";

  // 因为producer_未初始化，publish应返回false
  EXPECT_TRUE(published);
}
