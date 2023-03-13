
#ifndef STONE_EVENTLOOP_H
#define STONE_EVENTLOOP_H

#include <functional>
#include <memory>
#include <vector>
#include <mutex>
#include <thread>

namespace stone
{

    class Channel;
    class Poller;

    class EventLoop
    {
    public:
        using Functor = std::function<void()>;

        EventLoop();
        ~EventLoop();

        void loop();

        void quit();

        void runInLoop(Functor cb);

        void queueInLoop(Functor cb);

        size_t queueSize() const;

        void wakeup();
        void updateChannel(Channel *channel);
        void removeChannel(Channel *channel);
        bool hasChannel(Channel *channel);

        void assertInLoopThread()
        {
            if (!isInLoopThread())
            {
                abortNotInLoopThread();
            }
        }
        bool isInLoopThread() const { return threadId_ == std::this_thread::get_id(); }

        bool eventHandling() const { return eventHandling_; }

        static EventLoop *getEventLoopOfCurrentThread();

    private:
        void abortNotInLoopThread();
        void handleRead(); // waked up
        void doPendingFunctors();

        using ChannelList = std::vector<Channel *>;

        bool looping_; /* atomic */
        bool quit_;
        bool eventHandling_;          /* atomic */
        bool callingPendingFunctors_; /* atomic */
        int64_t iteration_;
        std::thread::id threadId_;
        std::unique_ptr<Poller> poller_;
        int wakeupFd_;
        std::unique_ptr<Channel> wakeupChannel_;

        ChannelList activeChannels_;
        Channel *currentActiveChannel_;

        std::mutex mutex_;
        std::vector<Functor> pendingFunctors_;
    };

}

#endif
