#pragma once

#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
#include <vector>

using namespace boost;

namespace hook_event::event {

// 定义事件类型
enum EnumEventType {
  MatchStart = 0,         /// 比赛开始
  MatchEnd,               /// 比赛结束
  CameraStream,           /// 摄像头数据流
  BallPosition,           /// 球位置
  PredTrackBallPosition,  /// 预测球轨迹
  RealTrackBallPosition,  /// 实际球轨迹
  ShuttlecockPosition,    /// 击球点位置
};

class EventMessage {
 public:
  virtual void matchStartCallback() {};
  virtual void matchEndCallback() {};
  virtual void cameraStreamCallback(
      const cv::Mat &leftFrame, const cv::Mat &rightFrame) {};
  virtual void ballPositionCallback(
      const cv::Point2f &leftPos, const cv::Point2f &rightPos) {};
  // virtual void predTrackBallPositionCallback(const
  // std::vector<cv::Point3f> &pos) {}; virtual void
  // realTrackBallPositionCallback(const
  // std::vector<cv::Point3f> &pos) {}; virtual void
  // shuttlecockPositionCallback(const cv::Point3f &pos) {};
};

// 事件管理器
class EventManager {
 public:
  EventManager()
      : io_context_(),
        work_guard_(boost::asio::make_work_guard(io_context_)),
        running_(false) {}

  ~EventManager() {
    stop();
  }

  void start() {
    if (running_.load()) return;
    running_.store(true);
    thread_ = std::thread([this] { io_context_.run(); });
  }

  void stop() {
    if (!running_.load()) return;
    running_.store(false);
    work_guard_.reset();
    io_context_.stop();
    if (thread_.joinable()) thread_.join();
  }

  void addCallback(std::shared_ptr<EventMessage> cb) {
    callbacks_.push_back(cb);

    // 注册事件
    matchStartSignal_.connect([cb]() { cb->matchStartCallback(); });
    matchEndSignal_.connect([cb]() { cb->matchEndCallback(); });
    cameraStreamSignal_.connect(
        [cb](const cv::Mat &l, const cv::Mat &r) { cb->cameraStreamCallback(l, r); });
    ballPositionSignal_.connect([cb](const cv::Point2f &l, const cv::Point2f &r) {
      cb->ballPositionCallback(l, r);
    });
    // predTrackBallSignal_.connect([cb](const std::vector<cv::Point3f> &pos) {
    //   cb->predTrackBallPositionCallback(pos);
    // });
    // realTrackBallSignal_.connect([cb](const std::vector<cv::Point3f> &pos) {
    //   cb->realTrackBallPositionCallback(pos);
    // });
    // shuttlecockSignal_.connect(
    //     [cb](const cv::Point3f &pos) { cb->shuttlecockPositionCallback(pos); });
  }

  // 触发事件函数
  void emit(EnumEventType type) {
    if (type == EnumEventType::MatchStart)
      io_context_.post([this] { matchStartSignal_(); });
    else if (type == EnumEventType::MatchEnd)
      io_context_.post([this] { matchEndSignal_(); });
    else
      throw std::runtime_error("Invalid event type for no-arg emit");
  }

  void emit(EnumEventType type, const cv::Mat &leftFrame, const cv::Mat &rightFrame) {
    if (type == EnumEventType::CameraStream)
      io_context_.post([this, leftFrame, rightFrame] {
        cameraStreamSignal_(leftFrame, rightFrame);
      });
    else
      throw std::runtime_error("Invalid event type for Mat emit");
  }

  void emit(
      EnumEventType type, const cv::Point2f &leftPos, const cv::Point2f &rightPos) {
    if (type == EnumEventType::BallPosition)
      io_context_.post(
          [this, leftPos, rightPos] { ballPositionSignal_(leftPos, rightPos); });
    else
      throw std::runtime_error("Invalid event type for Point2f emit");
  }

  void emit(EnumEventType type, const std::vector<cv::Point3f> &pos) {
    if (type == EnumEventType::PredTrackBallPosition)
      io_context_.post([this, pos] { predTrackBallSignal_(pos); });
    else if (type == EnumEventType::RealTrackBallPosition)
      io_context_.post([this, pos] { realTrackBallSignal_(pos); });
    else
      throw std::runtime_error("Invalid event type for vector<Point3f> emit");
  }

  void emit(EnumEventType type, const cv::Point3f &pos) {
    if (type == EnumEventType::ShuttlecockPosition)
      io_context_.post([this, pos] { shuttlecockSignal_(pos); });
    else
      throw std::runtime_error("Invalid event type for Point3f emit");
  }

  size_t poll() {
    return io_context_.poll();
  }

 private:
  asio::io_context io_context_;
  asio::executor_work_guard<asio::io_context::executor_type> work_guard_;
  std::thread thread_;
  std::atomic<bool> running_;
  std::vector<std::shared_ptr<EventMessage>> callbacks_;

  // 信号定义
  signals2::signal<void()> matchStartSignal_;
  signals2::signal<void()> matchEndSignal_;
  signals2::signal<void(const cv::Mat &, const cv::Mat &)> cameraStreamSignal_;
  signals2::signal<void(const cv::Point2f &, const cv::Point2f &)> ballPositionSignal_;
  signals2::signal<void(const std::vector<cv::Point3f> &)> predTrackBallSignal_;
  signals2::signal<void(const std::vector<cv::Point3f> &)> realTrackBallSignal_;
  signals2::signal<void(const cv::Point3f &)> shuttlecockSignal_;
};

};  // namespace hook_event::event