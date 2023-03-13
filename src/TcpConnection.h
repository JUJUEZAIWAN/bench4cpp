

#ifndef STONE_TCPCONNECTION_H
#define STONE_TCPCONNECTION_H

#include "EventLoop.h"
#include "Buffer.h"
#include "CallBack.h"
#include "Channel.h"
#include <thread>

namespace stone
{

    class TcpConnection : public std::enable_shared_from_this<TcpConnection>
    {
    public:
        TcpConnection(EventLoop *loop,
                      int sockfd);
        ~TcpConnection();

        EventLoop *getLoop() const { return loop_; }
        int fd() const { return channel_->fd(); }

        bool connected() const { return state_ == kConnected; }
 
        void send(const string &message);

        void forceClose();

  

        void setConnectionCallback(const ConnectionCallback &cb)
        {
            connectionCallback_ = cb;
        }

        void setMessageCallback(const MessageCallback &cb)
        {
            messageCallback_ = cb;
        }

 
        void setCloseCallback(const CloseCallback &cb)
        {
            closeCallback_ = cb;
        }

         
        void connectEstablished();  
       
        void connectDestroyed();  

    private:
        enum StateE
        {
            kDisconnected,
            kConnecting,
            kConnected,
            kDisconnecting
        };
        void handleRead();
        void handleWrite();
        void handleClose();
        void handleError();

        void sendInLoop(const string &message);
        void sendInLoop(const void *message, size_t len);

        void forceCloseInLoop();
        void setState(StateE s) { state_ = s; }

        EventLoop *loop_;
        StateE state_;
        bool reading_;
        int socket_;
        std::unique_ptr<Channel> channel_;
        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        CloseCallback closeCallback_;
        Buffer inputBuffer_;
        Buffer outputBuffer_;
    };

}

#endif