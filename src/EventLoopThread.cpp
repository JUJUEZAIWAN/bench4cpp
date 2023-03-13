
#include "EventLoopThread.h"

#include "EventLoop.h"

using namespace stone;

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb)
    : loop_(NULL),
      exiting_(false),
      thread_([this]
              { threadFunc(); }),
      callback_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
  exiting_ = true;
  if (loop_ != nullptr) // not 100% race-free, eg. threadFunc could be running callback_.
  {
    // still a tiny chance to call destructed object, if threadFunc exits just now.
    // but when EventLoopThread destructs, usually programming is exiting anyway.
    loop_->quit();
    thread_.join();
  }
}

EventLoop *EventLoopThread::startLoop()
{
  
  EventLoop *loop = nullptr;
  {
    std::unique_lock<std::mutex> lock(mutex_);
    while (loop_ == nullptr)
    {
      cond_.wait(lock);
    }
    loop = loop_;
  }

  return loop;
}

void EventLoopThread::threadFunc()
{
  EventLoop loop;

  if (callback_)
  {
    callback_(&loop);
  }

  {
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = &loop;
    cond_.notify_one();
  }

  loop.loop();
 
  std::unique_lock<std::mutex> lock(mutex_);
  loop_ = NULL;
}
