#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <eventpp/hetereventqueue.h>
#include <opencv2/opencv.hpp>

using EQ = eventpp::HeterEventQueue<
    int,
    eventpp::HeterTuple<
        void(int),
        void(const cv::Mat &, const cv::Mat &),
        void(const cv::Point2f &, const cv::Point2f &),
        void(const std::vector<cv::Point3f> &),
        void(const cv::Point3f &)>>;

// 定义事件类型
enum EventType
{
    MatchStart = 0,        /// 比赛开始
    MatchEnd,              /// 比赛结束
    CameraStream,          /// 摄像头数据流
    BallPosition,          /// 球位置
    PredTrackBallPosition, /// 预测球轨迹
    RealTrackBallPosition, /// 实际球轨迹
    ShuttlecockPosition,   /// 击球点位置
};

class EventMessage
{
public:
    virtual void matchStartCallback() {};
    virtual void matchEndCallback() {};
    virtual void cameraStreamCallback(const cv::Mat &leftFrame, const cv::Mat &rightFrame) {};
    virtual void ballPositionCallback(const cv::Point2f &leftPos, const cv::Point2f &rightPos) {};
    virtual void predTrackBallPositionCallback(const std::vector<cv::Point3f> &pos) {};
    virtual void realTrackBallPositionCallback(const std::vector<cv::Point3f> &pos) {};
    virtual void shuttlecockPositionCallback(const cv::Point3f &pos) {};
};

// 事件管理器
class EventManager
{
public:
    EventManager(const int32_t thread_interval = 1) : events(), running(false), thread_interval(thread_interval) {}
    ~EventManager()
    {
        stop();
    }

public:
    void start()
    {
        if (running.load())
            return;

        running.store(true);
        thread = std::thread(&EventManager::run, this);
    }

    void stop()
    {
        if (!running.load())
            return;

        running.store(false);
        thread.join();
    }
    bool emptyQueue() { return queue.emptyQueue(); }

    void addCallback(std::shared_ptr<EventMessage> cb)
    {
        events.push_back(cb);
        queue.appendListener(EventType::MatchStart, [cb](int)
                             { cb->matchStartCallback(); });
        queue.appendListener(EventType::MatchEnd, [cb](int)
                             { cb->matchEndCallback(); });
        queue.appendListener(EventType::CameraStream, [cb](const cv::Mat &leftFrame, const cv::Mat &rightFrame)
                             { cb->cameraStreamCallback(leftFrame, rightFrame); });
        queue.appendListener(EventType::BallPosition, [cb](const cv::Point2f &leftPos, const cv::Point2f &rightPos)
                             { cb->ballPositionCallback(leftPos, rightPos); });
        queue.appendListener(EventType::PredTrackBallPosition, [cb](const std::vector<cv::Point3f> &pos)
                             { cb->predTrackBallPositionCallback(pos); });
        queue.appendListener(EventType::RealTrackBallPosition, [cb](const std::vector<cv::Point3f> &pos)
                             { cb->realTrackBallPositionCallback(pos); });
        queue.appendListener(EventType::ShuttlecockPosition, [cb](const cv::Point3f &pos)
                             { cb->shuttlecockPositionCallback(pos); });
    }

    void emit(EventType type)
    {
        if (type == EventType::MatchStart || type == EventType::MatchEnd)
            queue.enqueue(type, 0);
        else
            throw std::runtime_error("Invalid event type");
    }

    void emit(EventType type, const cv::Mat &leftFrame, const cv::Mat &rightFrame)
    {
        if (type == EventType::CameraStream)
            queue.enqueue(type, leftFrame, rightFrame);
        else
            throw std::runtime_error("Invalid event type");
    }

    void emit(EventType type, const cv::Point2f &leftPos, const cv::Point2f &rightPos)
    {
        if (type == EventType::BallPosition)
            queue.enqueue(type, leftPos, rightPos);
        else
            throw std::runtime_error("Invalid event type");
    }

    void emit(EventType type, const std::vector<cv::Point3f> &pos)
    {
        if (type == EventType::PredTrackBallPosition || type == EventType::RealTrackBallPosition)
            queue.enqueue(type, pos);
        else
            throw std::runtime_error("Invalid event type");
    }

    void emit(EventType type, const cv::Point3f &pos)
    {
        if (type == EventType::ShuttlecockPosition)
            queue.enqueue(type, pos);
        else
            throw std::runtime_error("Invalid event type");
    }

private:
    void run()
    {
        while (running.load())
        {
            try
            {
                bool hasEvent = queue.waitFor(std::chrono::milliseconds(1));
                if (!hasEvent)
                    continue;

                queue.process();
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
            }
        }
    }

private:
    EQ queue;
    std::thread thread;
    std::atomic<bool> running;
    std::vector<std::shared_ptr<EventMessage>> events;
    const int32_t thread_interval;
};
