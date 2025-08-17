#include <gtest/gtest.h>
#include "watch/publisher/factory_publisher.hpp"
#include "watch/publisher/kafka_publisher.hpp"

TEST(PublisherFactoryTest, CreateKafkaPublisher)
{
    KafkaPublisherConfig config;
    config.brokerHost = "localhost:9092";
    auto publisher = PublisherFactory::create(config);
    ASSERT_NE(publisher, nullptr);
    // 检查类型
    auto kafkaPtr = dynamic_cast<KafkaPublisher *>(publisher.get());
    ASSERT_NE(kafkaPtr, nullptr);
    // 可以继续测试 publish/create_topic 等接口的行为（可用 mock 或 stub）
}

TEST(PublisherFactoryTest, KafkaPublisherCreateTopicAndPublish)
{
    KafkaPublisherConfig config;
    config.brokerHost = "localhost:9092";
    auto publisher = PublisherFactory::create(config);
    ASSERT_NE(publisher, nullptr);
    // 创建topic
    bool topicCreated = publisher->create_topic("test_topic");
    EXPECT_TRUE(topicCreated);
    // 发送消息（此处不会真正投递到Kafka，仅测试接口调用）
    bool published = publisher->publish("test_topic", "hello world");
    // 因为producer_未初始化，publish应返回false
    EXPECT_FALSE(published);
}
