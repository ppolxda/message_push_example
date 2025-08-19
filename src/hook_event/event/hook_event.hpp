#pragma once

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
#include <vector>

#include "../publisher/base_publisher.hpp"
#include "base_event.hpp"

std::string encode_base64(const std::vector<unsigned char> &input) {
  using namespace boost::archive::iterators;
  using base64_enc_iterator = base64_from_binary<
      transform_width<std::vector<unsigned char>::const_iterator, 6, 8>>;

  std::stringstream os;
  std::copy(
      base64_enc_iterator(input.begin()),
      base64_enc_iterator(input.end()),
      std::ostream_iterator<char>(os));

  // Add padding if needed
  size_t num_pad = (3 - input.size() % 3) % 3;
  for (size_t i = 0; i < num_pad; ++i) {
    os.put('=');
  }
  return os.str();
}

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
        topic_image_(topic_image) {}

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
      std::string encoded = encode_base64(buf);
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