#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include <csignal>
#include <librdkafka/rdkafkacpp.h>
#include <nlohmann/json.hpp>

class KafkaLogProducer
{
public:
    KafkaLogProducer(const std::string &broker, const std::string &topic)
        : broker_(broker), topic_(topic), producer_(nullptr)
    {
        init();
    }

    ~KafkaLogProducer()
    {
        if (producer_)
        {
            producer_->flush(5000);
            delete producer_;
        }
    }

    void sendLogBatches(int num_batches, int batch_size)
    {
        for (int i = 0; i < num_batches; ++i)
        {
            std::cout << "Sending batch " << (i + 1) << "/" << num_batches << std::endl;
            for (int j = 0; j < batch_size; ++j)
            {
                auto log = generateLogMessage();
                sendMessage(log.dump());
            }
            producer_->flush(1000);
        }
    }

private:
    std::string broker_;
    std::string topic_;
    RdKafka::Producer *producer_;

    void init()
    {
        std::string errstr;
        RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

        if (conf->set("bootstrap.servers", broker_, errstr) != RdKafka::Conf::CONF_OK)
        {
            throw std::runtime_error("Failed to set broker: " + errstr);
        }

        producer_ = RdKafka::Producer::create(conf, errstr);
        if (!producer_)
        {
            throw std::runtime_error("Failed to create producer: " + errstr);
        }

        delete conf;
    }

    nlohmann::json generateLogMessage()
    {
        static std::vector<std::string> levels = {"INFO", "WARNING", "ERROR", "DEBUG"};
        static std::vector<std::string> messages = {
            "User login successful",
            "User login failed",
            "Database connection established",
            "Database connection failed",
            "Service started",
            "Service stopped",
            "Payment processed",
            "Payment failed"};

        static std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> level_dist(0, levels.size() - 1);
        std::uniform_int_distribution<int> msg_dist(0, messages.size() - 1);

        nlohmann::json log_entry;
        log_entry["level"] = levels[level_dist(rng)];
        log_entry["message"] = messages[msg_dist(rng)];
        log_entry["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                                     std::chrono::system_clock::now().time_since_epoch())
                                     .count();

        return log_entry;
    }

    void sendMessage(const std::string &payload)
    {
        RdKafka::ErrorCode resp = producer_->produce(
            topic_, RdKafka::Topic::PARTITION_UA,
            RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
            const_cast<char *>(payload.c_str()), payload.size(),
            nullptr, 0,
            0, nullptr, nullptr);

        if (resp != RdKafka::ERR_NO_ERROR)
        {
            std::cerr << "Produce failed: " << RdKafka::err2str(resp) << std::endl;
        }
    }
};
