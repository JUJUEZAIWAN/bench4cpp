/*
 * @Author: 欢乐水牛
 * @Date: 2023-03-10 17:18:29
 * @LastEditTime: 2023-03-13 16:53:49
 * @LastEditors: 欢乐水牛
 * @Description: 
 * @FilePath: /CPP/PRO/bench/src/Client.h
 * 
 */



#include "TcpConnection.h"
#include "Connector.h"

#include <functional>
#include <memory>
#include <atomic>
#include <thread>
#include <map>
#include <iostream>

#include <netinet/in.h>

namespace stone
{
    class EventLoop;
    class EventLoopThreadPool;
    class Connector;

    class Client
    {
    public:
        using ThreadInitCallback = std::function<void(EventLoop *)>;

        Client(EventLoop *loop, int numsocks, struct sockaddr_in serverAddr);
        ~Client();

        EventLoop *getLoop() const { return loop_; }

        void setThreadNum(int numThreads);
        
        void setThreadInitCallback(const ThreadInitCallback &cb)
        {
            threadInitCallback_ = cb;
        }
         

        void reconect()
        {
            connector_->connect(1);  
        }

        std::shared_ptr<EventLoopThreadPool> threadPool()
        {
            return threadPool_;
        }

        void start();

        void quit();

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

        

    private:
        /// Not thread safe, but in loop
        void newConnection(int sockfd);
        /// Thread safe.
        void removeConnection(const TcpConnectionPtr &conn);
        /// Not thread safe, but in loop
        void removeConnectionInLoop(const TcpConnectionPtr &conn);

        using ConnectionMap = std::map<int, TcpConnectionPtr>;

        EventLoop *loop_; // loop for Connector

        std::unique_ptr<Connector> connector_;

        std::shared_ptr<EventLoopThreadPool> threadPool_;
        CloseCallback closeCallback_;
        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;

        ThreadInitCallback threadInitCallback_;
        int nextConnId_;
        ConnectionMap connections_;
    };
}