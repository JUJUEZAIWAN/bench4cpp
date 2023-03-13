#include "Client.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "Connector.h"


#include <netinet/in.h>

using namespace stone;

Client::Client(EventLoop *loop, int numsocks, sockaddr_in serverAddr)
    : loop_(loop),
      connector_(new Connector(loop, numsocks, serverAddr)),
      threadPool_(new EventLoopThreadPool(loop)),
      nextConnId_(0)
{
    connector_->setNewConnectionCallback(
        [this](int sockfd)
        { newConnection(sockfd); });
}

Client::~Client()
{
    loop_->assertInLoopThread();
    for (auto &item : connections_)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(
            [conn]()
            { conn->connectDestroyed(); });
        conn.reset();
    }
}

void Client::setThreadNum(int numThreads)
{
    assert(0 <= numThreads);
    threadPool_->setThreadNum(numThreads);
}

void Client::start()
{
    threadPool_->start(threadInitCallback_);

    loop_->runInLoop(
        [this]()
        { connector_->connect(); });
}

void Client::newConnection(int sockfd)
{
    loop_->assertInLoopThread();
    EventLoop *ioLoop = threadPool_->getNextLoop();

    ++nextConnId_;

    TcpConnectionPtr conn(new TcpConnection(ioLoop,
                                            sockfd));
    connections_[sockfd] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback(
        [this](const TcpConnectionPtr &ptr)
        { removeConnection(ptr); }); // FIXME: unsafe
    ioLoop->runInLoop(
        [conn]()
        { conn->connectEstablished(); });
}

void Client::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->runInLoop(
        [this, conn]()
        { removeConnectionInLoop(conn); });
}

void Client::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    loop_->assertInLoopThread();

    size_t n = connections_.erase(conn->fd());
    (void)n;
    assert(n == 1);
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
        [conn]()
        { conn->connectDestroyed(); });
    reconect();     
}


void Client::quit()
{
    loop_->quit();
}