 
#include "Connector.h"

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <sys/uio.h>  // readv


using namespace stone;

Connector::Connector(EventLoop *loop, int numsocks,  sockaddr_in serverAddr)
    : loop_(loop), numsocks_(numsocks),
      poller_(Poller::newDefaultPoller(loop)),
      serverAddr_(serverAddr)
{
}

Connector::~Connector()
{
    // do nothing
}

void Connector::connect(int numsocks )
{
    if (numsocks > 0)
        numsocks_ = numsocks;
    for (int i = 0; i < numsocks_; i++)
    {
        
        int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
        int ret = ::connect(sockfd, (struct sockaddr *)&serverAddr_, sizeof(serverAddr_));
        int savedErrno = (ret == 0) ? 0 : errno;
        switch (savedErrno)
        {
        case 0:
        case EINPROGRESS:
        case EINTR:
            connecting(sockfd);
            break;
        case EISCONN:
        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            retry(sockfd);
            break;

        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
            ::close(sockfd);
            break;

        default:
            ::close(sockfd);
            break;
        }
    }
}




/*
    add the socket to poller
    set the write callback
    enable write
*/
void Connector::connecting(int sockfd)
{
     
    Channel *channel = new Channel(loop_, sockfd);
    channel->setWriteCallback([this, channel]()
                              { handleWrite(channel); });
    channel->enableWriting();

    poller_->updateChannel(channel);
}

void Connector::retry(int sockfd)
{
    ::close(sockfd);
}




/*
    check if the socket is connected
    if error, retry
    if connected, call newConnectionCallback_ and remove the channel from poller
*/
void Connector::handleWrite(Channel *channel)
{
    int error = 0;
    socklen_t error_len = sizeof(error);
    int ret = ::getsockopt(channel->fd(), SOL_SOCKET, SO_ERROR, &error, &error_len);
    if( ret < 0 || error != 0)  // error
    {
        retry(channel->fd());
        return;
    }
      

    // connected
    if (newConnectionCallback_)
    {
        newConnectionCallback_(channel->fd());
    }
    channel->disableAll();
    channel->remove();
    poller_->removeChannel(channel);    
}
