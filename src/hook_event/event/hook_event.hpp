#pragma once

#include <eventpp/hetereventqueue.h>
#include <libbase64.h>

#include <iostream>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
#include <vector>

#include "../publisher/base_publisher.hpp"
#include "base_event.hpp"

class HookEventPublisher : public EventMessage {
 public:
  HookEventPublisher(
      std::shared_ptr<BasePublisher> publisher,
      const std::string topic = "test",
      const std::string topic_image = "test_image")
      : game_id_(0),
        frame_id_(0),
        publisher_(publisher),
        topic_(topic),
        topic_image_(topic_image_) {}

  void matchStartCallback() override {
    nlohmann::json msg = {
        {"event", "match_start"},
        {"game_id", ++game_id_},
        {"frame_id", 0},
    };
    frame_id_.store(0);
    publisher_->publish(topic_, msg.dump());
  }

  void matchEndCallback() override {
    nlohmann::json msg = {
        {"event", "match_end"},
        {"game_id", game_id_.load()},
        {"frame_id", frame_id_.load()},
    };
    publisher_->publish(topic_, msg.dump());
  }

  void cameraStreamCallback(
      const cv::Mat &leftFrame, const cv::Mat &rightFrame) override {
    uint32_t frame_id_pre = ++frame_id_;

    auto encode_and_send = [this, frame_id_pre](
                               const cv::Mat &frame, const std::string &event_name) {
      std::vector<unsigned char> buf;
      cv::imencode(".png", frame, buf);
      size_t outlen = 4 * ((buf.size() + 2) / 3);
      std::string encoded(outlen, '\0');
      base64_encode(
          reinterpret_cast<const char *>(buf.data()),
          buf.size(),
          &encoded[0],
          &outlen,
          0);
      assert(!encoded.empty());

      // 推送内容
      nlohmann::json msg = {
          {"event", event_name},
          {"game_id", game_id_.load()},
          {"frame_id", frame_id_pre},
          {"data", encoded},
      };
      publisher_->publish(topic_image_, msg.dump());
    };
    encode_and_send(leftFrame, "camera_stream_left");
    encode_and_send(rightFrame, "camera_stream_right");
  }

  void ballPositionCallback(
      const cv::Point2f &leftPos, const cv::Point2f &rightPos) override {
    nlohmann::json msg = {
        {"event", "ball_position"},
        {"game_id", game_id_.load()},
        {"frame_id", frame_id_.load()},
        {"left", {leftPos.x, leftPos.y}},
        {"right", {rightPos.x, rightPos.y}},
    };
    publisher_->publish(topic_, msg.dump());
  }

  // void predTrackBallPositionCallback(const std::vector<cv::Point3f>
  // &pos) override
  // {
  //     nlohmann::json arr;
  //     for (const auto &p : pos)
  //         arr.push_back({p.x, p.y, p.z});
  //     nlohmann::json msg = {{"event", "pred_track_ball_position"},
  //     {"positions", arr}}; publisher_->publish(topic_, msg.dump());
  // }

  // void realTrackBallPositionCallback(const std::vector<cv::Point3f>
  // &pos) override
  // {
  //     nlohmann::json arr;
  //     for (const auto &p : pos)
  //         arr.push_back({p.x, p.y, p.z});
  //     nlohmann::json msg = {{"event", "real_track_ball_position"},
  //     {"positions", arr}}; publisher_->publish(topic_, msg.dump());
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
  const std::string topic_image_;
  std::atomic<uint32_t> game_id_{0};
  std::atomic<uint32_t> frame_id_{0};
  std::shared_ptr<BasePublisher> publisher_;
};