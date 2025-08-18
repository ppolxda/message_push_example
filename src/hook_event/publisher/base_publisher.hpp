#pragma once
#include <cstdint>
#include <map>
#include <memory>
#include <string>

class BasePublisher {
 public:
  virtual ~BasePublisher() = default;

  // 发布消息到指定主题/队列
  virtual bool publish(const std::string &topic, const std::string &message) {
    return publish(
        topic, reinterpret_cast<const unsigned char *>(message.data()), message.size());
  }

  // 发布消息到指定主题/队列
  virtual bool publish(
      const std::string &topic, const unsigned char *message, const size_t size) = 0;

  // 创建主题/队列，部分中间件支持动态创建
  virtual bool create_topic(
      const std::string &topic,
      const std::map<std::string, std::string> &options = {}) = 0;
};
