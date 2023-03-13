 
 
#include "Poller.h"
 
#include <sys/epoll.h>
namespace stone
{
    class EPoller : public Poller
    {
    public:
        EPoller(EventLoop *loop);
        ~EPoller() override;

        void poll(int timeoutMs, ChannelList *activeChannels) override;
        void updateChannel(Channel *channel) override;
        void removeChannel(Channel *channel) override;

    private:
        static const int kInitEventListSize = 16;

        void fillActiveChannels(int numEvents,
                                ChannelList *activeChannels) ;
        void update(int operation, Channel *channel);

        using EventList = std::vector<struct epoll_event>;

        int epollfd_;
        EventList events_;
    };
}
