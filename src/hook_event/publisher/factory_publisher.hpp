#pragma once
#include <map>
#include <memory>
#include <string>

#include "base_publisher.hpp"
#include "kafka_publisher.hpp"

namespace hook_event::publisher {

// KafkaPublisher 工厂
class PublisherFactory {
 public:
  static std::shared_ptr<BasePublisher> createKafkaPublisher(
      const KafkaPublisherConfig &config) {
    return std::make_shared<KafkaPublisher>(config);
  }
};

};  // namespace hook_event::publisher