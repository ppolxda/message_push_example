#pragma once

#include <eventpp/hetereventqueue.h>

#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
#include <vector>

using EQ = eventpp::HeterEventQueue<
    int,
    eventpp::HeterTuple<
        void(int),
        void(const cv::Mat &, const cv::Mat &),
        void(const cv::Point2f &, const cv::Point2f &),
        void(const std::vector<cv::Point3f> &),
        void(const cv::Point3f &)>>;

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
  EventManager(const int32_t thread_interval = 1)
      : events_(), running_(false), thread_interval_(thread_interval) {}
  ~EventManager() {
    stop();
  }

 public:
  void start() {
    if (running_.load()) return;

    running_.store(true);
    thread_ = std::thread(&EventManager::run, this);
  }

  void stop() {
    if (!running_.load()) return;

    running_.store(false);
    thread_.join();
  }
  bool emptyQueue() {
    return queue_.emptyQueue();
  }

  void addCallback(std::shared_ptr<EventMessage> cb) {
    events_.push_back(cb);
    queue_.appendListener(
        EnumEventType::MatchStart, [cb](int) { cb->matchStartCallback(); });
    queue_.appendListener(
        EnumEventType::MatchEnd, [cb](int) { cb->matchEndCallback(); });
    queue_.appendListener(
        EnumEventType::CameraStream,
        [cb](const cv::Mat &leftFrame, const cv::Mat &rightFrame) {
          cb->cameraStreamCallback(leftFrame, rightFrame);
        });
    queue_.appendListener(
        EnumEventType::BallPosition,
        [cb](const cv::Point2f &leftPos, const cv::Point2f &rightPos) {
          cb->ballPositionCallback(leftPos, rightPos);
        });
    // queue_.appendListener(EventType::PredTrackBallPosition,
    // [cb](const std::vector<cv::Point3f> &pos)
    //                       {
    //                       cb->predTrackBallPositionCallback(pos);
    //                       });
    // queue_.appendListener(EventType::RealTrackBallPosition,
    // [cb](const std::vector<cv::Point3f> &pos)
    //                       {
    //                       cb->realTrackBallPositionCallback(pos);
    //                       });
    // queue_.appendListener(EventType::ShuttlecockPosition,
    // [cb](const cv::Point3f &pos)
    //                       {
    //                       cb->shuttlecockPositionCallback(pos);
    //                       });
  }

  void emit(EnumEventType type) {
    if (type == EnumEventType::MatchStart || type == EnumEventType::MatchEnd)
      queue_.enqueue(type, 0);
    else
      throw std::runtime_error("Invalid event type");
  }

  void emit(EnumEventType type, const cv::Mat &leftFrame, const cv::Mat &rightFrame) {
    if (type == EnumEventType::CameraStream)
      queue_.enqueue(type, leftFrame, rightFrame);
    else
      throw std::runtime_error("Invalid event type");
  }

  void emit(
      EnumEventType type, const cv::Point2f &leftPos, const cv::Point2f &rightPos) {
    if (type == EnumEventType::BallPosition)
      queue_.enqueue(type, leftPos, rightPos);
    else
      throw std::runtime_error("Invalid event type");
  }

  void emit(EnumEventType type, const std::vector<cv::Point3f> &pos) {
    if (type == EnumEventType::PredTrackBallPosition ||
        type == EnumEventType::RealTrackBallPosition)
      queue_.enqueue(type, pos);
    else
      throw std::runtime_error("Invalid event type");
  }

  void emit(EnumEventType type, const cv::Point3f &pos) {
    if (type == EnumEventType::ShuttlecockPosition)
      queue_.enqueue(type, pos);
    else
      throw std::runtime_error("Invalid event type");
  }

 private:
  void run() {
    while (running_.load()) {
      try {
        bool hasEvent = queue_.waitFor(std::chrono::milliseconds(1));
        if (!hasEvent) continue;

        queue_.process();
      } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
      }
    }
  }

 private:
  EQ queue_;
  std::thread thread_;
  std::atomic<bool> running_;
  std::vector<std::shared_ptr<EventMessage>> events_;
  const int32_t thread_interval_;
};
