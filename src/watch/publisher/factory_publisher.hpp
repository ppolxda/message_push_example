#pragma once
#include <memory>
#include <string>
#include <map>
#include "base_publisher.hpp"
#include "kafka_publisher.hpp"

// KafkaPublisher 工厂
class PublisherFactory
{
public:
    static std::unique_ptr<KafkaPublisher> create(const KafkaPublisherConfig &config)
    {
        return std::make_unique<KafkaPublisher>(config);
    }
};
