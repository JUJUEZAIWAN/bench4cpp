/*
 * @Author: 欢乐水牛
 * @Date: 2023-03-10 17:18:29
 * @LastEditTime: 2023-03-10 17:26:50
 * @LastEditors: 欢乐水牛
 * @Description: 
 * @FilePath: /CPP/PRO/bench/src/EventLoopThread.h
 * 
 */

#ifndef STONE_EVENTLOOPTHREAD_H
#define STONE_EVENTLOOPTHREAD_H

#include <condition_variable>
#include <functional>
#include <memory>
#include <thread>
#include <string>

namespace stone
{
  using std::string;
  class EventLoop;

  class EventLoopThread
  {
  public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback());
    ~EventLoopThread();
    EventLoop *startLoop();

  private:
    void threadFunc();

    EventLoop *loop_;
    bool exiting_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
  };

}

#endif
