#pragma once
#include <librdkafka/rdkafkacpp.h>

#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>

#include "base_publisher.hpp"

namespace hook_event::publisher {

struct KafkaPublisherConfig {
  // client config
  std::string bootstrapServers = "localhost:9092";
  // std::string groupId = "mpl";
  std::string clientId = "mpl";
  std::string acks = "all";
  std::string securityProtocol =
      "";                           // "PLAINTEXT", "SSL", "SASL_PLAINTEXT", "SASL_SSL"
  std::string saslMechanisms = "";  // "PLAIN", "SCRAM-SHA-256"
  std::string saslUsername = "";
  std::string saslPassword = "";
  std::string lingerMs = "10";
  std::string compressionType = "none";  // none、gzip、snappy、lz4、zstd
  std::string queueBufferingMaxMessages = "1000000";
  std::string queueBufferingKbytes = "1048576";
  std::string queueBufferingMaxMs = "1000";
  std::string messageSendMaxRetries = "3";
  std::string messageMaxBytes = "10485760";  // 10m
  std::string messageTimeoutMs = "60000";
  std::string retryBackoffMs = "100";
  std::string requestTimeoutMs = "30000";
  // std::string socketTimeoutMs = "30000";
  // std::string sessionTimeoutMs = "10000";

  std::map<std::string, std::string> toMap() const {
    std::map<std::string, std::string> m;
    if (!clientId.empty()) m["client.id"] = clientId;

    m["bootstrap.servers"] = bootstrapServers;
    // m["group.id"] = groupId;
    m["acks"] = acks;
    m["security.protocol"] = securityProtocol;
    m["sasl.mechanisms"] = saslMechanisms;
    m["sasl.username"] = saslUsername;
    m["sasl.password"] = saslPassword;
    m["linger.ms"] = lingerMs;
    m["compression.type"] = compressionType;
    m["queue.buffering.max.messages"] = queueBufferingMaxMessages;
    m["queue.buffering.max.kbytes"] = queueBufferingKbytes;
    m["queue.buffering.max.ms"] = queueBufferingMaxMs;
    m["message.max.bytes"] = messageMaxBytes;
    m["message.send.max.retries"] = messageSendMaxRetries;
    m["message.timeout.ms"] = messageTimeoutMs;
    m["retry.backoff.ms"] = retryBackoffMs;
    m["request.timeout.ms"] = requestTimeoutMs;
    // m["socket.timeout.ms"] = socketTimeoutMs;
    // m["session.timeout.ms"] = sessionTimeoutMs;
    return m;
  }
};

class KafkaPublisher : public BasePublisher {
 public:
  KafkaPublisher(KafkaPublisherConfig cfg)
      : producer_(nullptr), config_(cfg), running_(false) {
    start();
  }
  ~KafkaPublisher() override {
    stop();
  }

  // Kafka不直接支持主题创建，通常由服务端自动创建或用admin client。这里只做占位实现。
  bool create_topic(
      const std::string &topic,
      const std::map<std::string, std::string> &options = {}) override {
    // RdKafka::Topic *topic = RdKafka::Topic::create(producer_, topic, nullptr,
    // errstr); if (!topic)
    // {
    //     std::cerr << "Failed to create topic: " << errstr << std::endl;
    //     exit(1);
    // }
    throw std::runtime_error("not supported");
  }

  bool publish(
      const std::string &topic,
      const unsigned char *message,
      const size_t size) override {
    if (!running_.load() || !producer_) return false;

    RdKafka::ErrorCode resp = producer_->produce(
        topic,
        RdKafka::Topic::PARTITION_UA,
        RdKafka::Producer::RK_MSG_COPY,
        const_cast<unsigned char *>(message),
        size,
        nullptr,
        0,
        0,
        nullptr,
        nullptr);

    if (resp != RdKafka::ERR_NO_ERROR) {
      std::cerr << "Produce failed: " << RdKafka::err2str(resp) << std::endl;
      return false;
    }
    return true;
  }

 private:
  // config: {"bootstrap.servers": "host1:9092,host2:9092"}
  void start() {
    if (running_.load()) return;

    running_.store(true);
    std::string errstr;
    std::unique_ptr<RdKafka::Conf> conf = std::unique_ptr<RdKafka::Conf>(
        RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
    auto config = config_.toMap();
    for (const auto &kv : config) {
      if (kv.second.empty()) continue;

      if (conf->set(kv.first, kv.second, errstr) != RdKafka::Conf::CONF_OK) {
        std::cerr << "Kafka config error: " << errstr << std::endl;
        running_.store(false);
        throw std::runtime_error(errstr);
      }
    }

    producer_ = RdKafka::Producer::create(conf.get(), errstr);
    if (!producer_) {
      std::cerr << "Failed to create producer: " << errstr << std::endl;
      running_.store(false);
      throw std::runtime_error(errstr);
    }

    thread_ = std::thread([this]() {
      while (running_.load()) {
        producer_->poll(100);  // 每100ms poll一次
      }
    });
  }

  void stop() {
    if (!running_.load()) return;

    running_.store(false);
    thread_.join();

    if (producer_) {
      producer_->flush(5000);
      delete producer_;
      producer_ = nullptr;
    }
  }

 private:
  std::atomic<bool> running_;
  std::thread thread_;
  RdKafka::Producer *producer_;
  KafkaPublisherConfig config_;
  std::string topic_;
};

};  // namespace hook_event::publisher