
#include "TcpConnection.h"
#include "Buffer.h"
#include "Channel.h"
#include "EventLoop.h"

#include <errno.h>
#include <memory>
#include <cassert>
#include <algorithm>

#include <sys/socket.h>
#include <sys/uio.h> // readv
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <sys/uio.h>

using namespace stone;

TcpConnection::TcpConnection(EventLoop *loop,
                             int sockfd)
    : loop_(loop),
      state_(kConnecting),
      reading_(true),
      socket_(sockfd),
      channel_(new Channel(loop, sockfd))
{
    channel_->setReadCallback(
        [this]()
        { handleRead(); });
    channel_->setWriteCallback(
        [this]()
        { handleWrite(); });
    channel_->setCloseCallback(
        [this]()
        { handleClose(); });

    channel_->setErrorCallback(
        [this]()
        { handleError(); });
}

TcpConnection::~TcpConnection()
{
    assert(state_ == kDisconnected);

    ::close(socket_);
}

void TcpConnection::send(const string &message)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(message);
        }
        else
        {
            loop_->runInLoop([this, message]()
                             { sendInLoop(message); });
        }
    }
}

void TcpConnection::sendInLoop(const string &message)
{
    sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(const void *data, size_t len)
{
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;

    if (state_ == kDisconnected)
    {

        return;
    }
    // if no thing in output queue, try writing directly
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = ::write(channel_->fd(), data, len);

        if (nwrote >= 0)
        {
            remaining = len - nwrote;
        }
        else // nwrote < 0
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {

                if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
                {
                    faultError = true;
                    
                }
            }
        }
    }

    assert(remaining <= len);
    if (!faultError && remaining > 0)
    {
        size_t oldLen = outputBuffer_.readableBytes();

        outputBuffer_.append(static_cast<const char *>(data) + nwrote, remaining);
        if (!channel_->isWriting())
        {
            channel_->enableWriting();
        }
    }
    channel_->enableReading();
}

void TcpConnection::forceClose()
{
    if (state_ == kConnected || state_ == kDisconnecting)
    {
        setState(kDisconnecting);
        loop_->queueInLoop([this]()
                           { forceCloseInLoop(); });
    }
}

void TcpConnection::forceCloseInLoop()
{
    loop_->assertInLoopThread();
    if (state_ == kConnected || state_ == kDisconnecting)
    {
        // as if we received 0 byte in handleRead();
        handleClose();
    }
}

void TcpConnection::connectEstablished()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableWriting();

    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    if (state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll();

        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::handleRead()
{
    loop_->assertInLoopThread();
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0)
    {
        messageCallback_(shared_from_this(), &inputBuffer_);
    }
    else if (n == 0)
    {
        setState(kDisconnecting);
        forceCloseInLoop();
    }
    else
    {

        errno = savedErrno; // FIXME : deal with errno
        setState(kDisconnecting);
        forceCloseInLoop();
    }
}

void TcpConnection::handleWrite()
{
    loop_->assertInLoopThread();
    if (channel_->isWriting())
    {
        ssize_t n = ::write(channel_->fd(), outputBuffer_.peek(), outputBuffer_.readableBytes());
        if (n > 0)
        {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0)
            {
                channel_->disableWriting();

                if (state_ == kDisconnecting)
                {
                    forceCloseInLoop();
                }
            }
        }
        else if (n == 0) // server close
        {
            setState(kDisconnecting);
            forceCloseInLoop();
        }
        else
        {

            // FIXME : deal with errno
            setState(kDisconnecting);
            forceCloseInLoop();
        }
    }
}

void TcpConnection::handleClose()
{
    loop_->assertInLoopThread();

    assert(state_ == kConnected || state_ == kDisconnecting);

    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr guardThis(shared_from_this());
    connectionCallback_(guardThis);
    closeCallback_(guardThis);
}

void TcpConnection::handleError()
{

    int error = 0;
    socklen_t error_len = sizeof(error);
    int ret = ::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &error, &error_len);
    if (ret < 0)  
    {
        std::cout << "TcpConnection::handleError() SO_ERROR = " << error << std::endl;
        // FIXME : deal with errno
    }
}