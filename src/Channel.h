#ifndef STONE_CHANNEL_H
#define STONE_CHANNEL_H

 
#include <string>
#include <memory>
#include <functional>


namespace stone
{
    using std::string;

    class EventLoop;

    class Channel
    {
    public:
        using EventCallback = std::function<void()>;
        using ReadEventCallback = std::function<void()>;

        Channel(EventLoop *loop, int fd);
        ~Channel();

        void handleEvent();
        void setReadCallback(ReadEventCallback cb)
        {
            readCallback_ = std::move(cb);
        }
        void setWriteCallback(EventCallback cb)
        {
            writeCallback_ = std::move(cb);
        }
        void setCloseCallback(EventCallback cb)
        {
            closeCallback_ = std::move(cb);
        }
        void setErrorCallback(EventCallback cb)
        {
            errorCallback_ = std::move(cb);
        }

        /// Tie this channel to the owner object managed by shared_ptr,
        /// prevent the owner object being destroyed in handleEvent.
        void tie(const std::shared_ptr<void> &);

        int fd() const { return fd_; }
        int events() const { return events_; }
        void set_revents(int revt) { revents_ = revt; } // used by pollers
        // int revents() const { return revents_; }
        bool isNoneEvent() const { return events_ == kNoneEvent; }

        void enableReading()
        {
            events_ |= kReadEvent;
            update();
        }
        void disableReading()
        {
            events_ &= ~kReadEvent;
            update();
        }
        void enableWriting()
        {
            events_ |= kWriteEvent;
            update();
        }
        void disableWriting()
        {
            events_ &= ~kWriteEvent;
            update();
        }
        void disableAll()
        {
            events_ = kNoneEvent;
            update();
        }
        bool isWriting() const { return events_ & kWriteEvent; }
        bool isReading() const { return events_ & kReadEvent; }

        // for Poller
        int index() { return index_; }
        void set_index(int idx) { index_ = idx; }

        EventLoop *ownerLoop() { return loop_; }
        void remove();

    private:
        void update();
        void handleEventWithGuard();

        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;

        EventLoop *loop_;
        const int fd_;
        int events_;
        int revents_; // it's the received event types of epoll or poll
        int index_;   // used by Poller.

        std::weak_ptr<void> tie_;
        bool tied_;
        bool eventHandling_;
        bool addedToLoop_;
        ReadEventCallback readCallback_;
        EventCallback writeCallback_;
        EventCallback closeCallback_;
        EventCallback errorCallback_;
    };

}

#endif