
#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include <csignal>eeeeeee
#include <librdkafka/rdkafkacpp.h>
#include <nlohmann/json.hpp>
#include "base_publisher.hpp"

struct KafkaPublisherConfig
{
    std::string brokerHost;
};

class KafkaPublisher : public BasePublisher
{
public:
    KafkaPublisher(KafkaPublisherConfig cfg) : producer_(nullptr), config_(cfg) {}
    ~KafkaPublisher() override
    {
        if (producer_)
        {
            producer_->flush(5000);
            delete producer_;
        }
    }

    // Kafka不直接支持主题创建，通常由服务端自动创建或用admin client。这里只做占位实现。
    bool create_topic(const std::string &topic, const std::map<std::string, std::string> &options = {}) override
    {
        // 可集成librdkafka Admin API实现真正的主题创建，这里简单返回true
        topic_ = topic;
        return true;
    }

    bool publish(const std::string &topic, const std::string &message) override
    {
        if (!producer_)
            return false;
        RdKafka::ErrorCode resp = producer_->produce(
            topic, RdKafka::Topic::PARTITION_UA,
            RdKafka::Producer::RK_MSG_COPY,
            const_cast<char *>(message.c_str()), message.size(),
            nullptr, 0,
            0, nullptr, nullptr);
        if (resp != RdKafka::ERR_NO_ERROR)
        {
            std::cerr << "Produce failed: " << RdKafka::err2str(resp) << std::endl;
            return false;
        }
        producer_->poll(0);
        return true;
    }

private:
    // config: {"bootstrap.servers": "host1:9092,host2:9092"}
    bool connect(const std::map<std::string, std::string> &config)
    {
        std::string errstr;
        RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
        for (const auto &kv : config)
        {
            if (conf->set(kv.first, kv.second, errstr) != RdKafka::Conf::CONF_OK)
            {
                std::cerr << "Kafka config error: " << errstr << std::endl;
                delete conf;
                return false;
            }
        }
        producer_ = RdKafka::Producer::create(conf, errstr);
        if (!producer_)
        {
            std::cerr << "Failed to create producer: " << errstr << std::endl;
            delete conf;
            return false;
        }
        delete conf;
        return true;
    }

    void disconnect()
    {
        if (producer_)
        {
            producer_->flush(5000);
            delete producer_;
            producer_ = nullptr;
        }
    }

private:
    RdKafka::Producer *producer_;
    KafkaPublisherConfig config_;
    std::string topic_;
};
