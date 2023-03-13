#ifndef STONE_CONNECTOR_H
#define STONE_CONNECTOR_H


#include "Channel.h"
#include "Poller.h"


#include <functional>
#include <memory>


#include <netinet/in.h>


namespace stone
{

    class EventLoop;
    class Connector
    {

    public:
        using NewConnectionCallback = std::function<void(int sockfd)>;
        Connector(EventLoop *loop,int numsocks,struct sockaddr_in serverAddr);
        ~Connector();

        void connect(int num=0 );

        void setNewConnectionCallback(const NewConnectionCallback &cb)
        {
            newConnectionCallback_ = cb;
        }

        void connecting(int sockfd);
        void retry(int sockfd);

    private:

        void handleWrite(Channel *channel);
        EventLoop *loop_;
        int numsocks_;
        std::unique_ptr<Poller> poller_;
        struct sockaddr_in serverAddr_;
        NewConnectionCallback newConnectionCallback_;
    };

};

#endif


