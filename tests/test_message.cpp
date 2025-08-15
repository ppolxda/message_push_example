#include <gtest/gtest.h>
#include <memory>
#include <atomic>

#include "watch/messages/message.hpp"

class TestEventMessage : public EventMessage
{
public:
    std::atomic<bool> matchStartCalled{false};
    std::atomic<bool> matchEndCalled{false};
    void matchStartCallback() override { matchStartCalled = true; }
    void matchEndCallback() override { matchEndCalled = true; }
};

TEST(EventManagerTest, MatchStartAndEndCallbacks)
{
    auto msg = std::make_shared<TestEventMessage>();
    EventManager manager;
    manager.addCallback(msg);
    manager.start();
    manager.emit(EventType::MatchStart);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(msg->matchStartCalled);
    manager.emit(EventType::MatchEnd);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(msg->matchEndCalled);
    manager.stop();
}

// int main(int argc, char **argv)
// {
//     ::testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }
