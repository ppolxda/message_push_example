#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <eventpp/hetereventqueue.h>
#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>
#include <libbase64.h>

#include "base_event.hpp"
#include "../publisher/base_publisher.hpp"

class HookEventPublisher : public EventMessage
{
public:
    HookEventPublisher(std::shared_ptr<BasePublisher> publisher, const std::string topic = "test")
        : publisher_(publisher), topic_(topic) {}

    void matchStartCallback() override
    {
        nlohmann::json msg = {{"event", "match_start"}};
        publisher_->publish(topic_, msg.dump());
    }

    void matchEndCallback() override
    {
        nlohmann::json msg = {{"event", "match_end"}};
        publisher_->publish(topic_, msg.dump());
    }

    void cameraStreamCallback(const cv::Mat &leftFrame, const cv::Mat &rightFrame) override
    {
        auto encode_and_send = [this](const cv::Mat &frame, const std::string &event_name)
        {
            std::vector<unsigned char> buf;
            cv::imencode(".png", frame, buf);
            size_t outlen = 4 * ((buf.size() + 2) / 3);
            std::string encoded(outlen, '\0');
            base64_encode(reinterpret_cast<const char *>(buf.data()), buf.size(), &encoded[0], &outlen, 0);
            assert(!encoded.empty());

            // 推送内容
            nlohmann::json msg = {{"event", event_name}, {"data", encoded}};
            publisher_->publish(topic_, msg.dump());
        };

        encode_and_send(leftFrame, "camera_stream_left");
        encode_and_send(rightFrame, "camera_stream_right");
    }

    void ballPositionCallback(const cv::Point2f &leftPos, const cv::Point2f &rightPos) override
    {
        nlohmann::json msg = {
            {"event", "ball_position"},
            {"left", {leftPos.x, leftPos.y}},
            {"right", {rightPos.x, rightPos.y}}};
        publisher_->publish(topic_, msg.dump());
    }

    // void predTrackBallPositionCallback(const std::vector<cv::Point3f> &pos) override
    // {
    //     nlohmann::json arr;
    //     for (const auto &p : pos)
    //         arr.push_back({p.x, p.y, p.z});
    //     nlohmann::json msg = {{"event", "pred_track_ball_position"}, {"positions", arr}};
    //     publisher_->publish(topic_, msg.dump());
    // }

    // void realTrackBallPositionCallback(const std::vector<cv::Point3f> &pos) override
    // {
    //     nlohmann::json arr;
    //     for (const auto &p : pos)
    //         arr.push_back({p.x, p.y, p.z});
    //     nlohmann::json msg = {{"event", "real_track_ball_position"}, {"positions", arr}};
    //     publisher_->publish(topic_, msg.dump());
    // }

    // void shuttlecockPositionCallback(const cv::Point3f &pos) override
    // {
    //     nlohmann::json msg = {
    //         {"event", "shuttlecock_position"},
    //         {"position", {pos.x, pos.y, pos.z}}};
    //     publisher_->publish(topic_, msg.dump());
    // }

private:
    const std::string topic_;
    std::shared_ptr<BasePublisher> publisher_;
};