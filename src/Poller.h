
#ifndef STONE_POLLER_H
#define STONE_POLLER_H


#include "EventLoop.h"

#include <map>
#include <vector>

namespace stone
{
    class Channel;
    

    class Poller
    {

    public:
        using ChannelList = std::vector<Channel *>;


        Poller(EventLoop *loop);
        virtual ~Poller();


        virtual void poll(int timeoutMs, ChannelList *activeChannels) = 0;

        virtual void updateChannel(Channel *channel) = 0;

        virtual void removeChannel(Channel *channel) = 0;

        virtual bool hasChannel(Channel *channel) const;

        static Poller *newDefaultPoller(EventLoop *loop);

        void assertInLoopThread() const
        {
            ownerLoop_->assertInLoopThread();
        }

    protected:
        using ChannelMap = std::map<int, Channel *>;
        ChannelMap channels_;

    private:

        EventLoop *ownerLoop_;
    };
}

#endif