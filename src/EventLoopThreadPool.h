


#ifndef STONE_EVENTLOOPTHREADPOOL_H
#define STONE_EVENTLOOPTHREADPOOL_H

#include "EventLoopThread.h"
#include <functional>
#include <memory>
#include <vector>

namespace stone
{

  class EventLoop;
  class EventLoopThread;

  class EventLoopThreadPool
  {
  public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;


    EventLoopThreadPool(EventLoop *baseLoop);
    ~EventLoopThreadPool();
    void setThreadNum(int numThreads) { numThreads_ = numThreads; }
    void start(const ThreadInitCallback &cb = ThreadInitCallback());


    EventLoop *getNextLoop();
    std::vector<EventLoop *> getAllLoops();

    bool started() const
    {
      return started_;
    }



  private:
    EventLoop *baseLoop_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop *> loops_;
  };

}

#endif
