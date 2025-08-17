#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <eventpp/hetereventqueue.h>
#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>

#include "message.hpp"
#include "../publisher/base_publisher.hpp"

class WatchEventMessagePublisher : public EventMessage
{
public:
    WatchEventMessagePublisher(std::unique_ptr<BasePublisher> publisher)
        : publisher_(publisher) {}

    void matchStartCallback() override
    {
        nlohmann::json msg = {{"event", "match_start"}};
        publisher_->publish(WatchEventMessagePublisher::topic_, msg.dump());
    }

    void matchEndCallback() override
    {
        nlohmann::json msg = {{"event", "match_end"}};
        publisher_->publish(WatchEventMessagePublisher::topic_, msg.dump());
    }

    void cameraStreamCallback(const cv::Mat &leftFrame, const cv::Mat &rightFrame) override
    {
        // 实际应用中建议只传递帧元数据或图片URL，避免直接传输大数据
        nlohmann::json msg = {{"event", "camera_stream"}, {"info", "frame received"}};
        publisher_->publish(WatchEventMessagePublisher::topic_, msg.dump());
    }

    void ballPositionCallback(const cv::Point2f &leftPos, const cv::Point2f &rightPos) override
    {
        nlohmann::json msg = {
            {"event", "ball_position"},
            {"left", {leftPos.x, leftPos.y}},
            {"right", {rightPos.x, rightPos.y}}};
        publisher_->publish(WatchEventMessagePublisher::topic_, msg.dump());
    }

    void predTrackBallPositionCallback(const std::vector<cv::Point3f> &pos) override
    {
        nlohmann::json arr;
        for (const auto &p : pos)
            arr.push_back({p.x, p.y, p.z});
        nlohmann::json msg = {{"event", "pred_track_ball_position"}, {"positions", arr}};
        publisher_->publish(WatchEventMessagePublisher::topic_, msg.dump());
    }

    void realTrackBallPositionCallback(const std::vector<cv::Point3f> &pos) override
    {
        nlohmann::json arr;
        for (const auto &p : pos)
            arr.push_back({p.x, p.y, p.z});
        nlohmann::json msg = {{"event", "real_track_ball_position"}, {"positions", arr}};
        publisher_->publish(WatchEventMessagePublisher::topic_, msg.dump());
    }

    void shuttlecockPositionCallback(const cv::Point3f &pos) override
    {
        nlohmann::json msg = {
            {"event", "shuttlecock_position"},
            {"position", {pos.x, pos.y, pos.z}}};
        publisher_->publish(WatchEventMessagePublisher::topic_, msg.dump());
    }

private:
    std::unique_ptr<BasePublisher> publisher_;
    static std::string WatchEventMessagePublisher::topic_;
};
