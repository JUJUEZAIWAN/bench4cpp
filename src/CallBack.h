#ifndef STONE_CALLBACK_H
#define STONE_CALLBACK_H


#include <functional>
#include <memory>


namespace stone{

    class TcpConnection;
    class Buffer;


    class TcpConnection;
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

    using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
    using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
    using MessageCallback = std::function<void(const TcpConnectionPtr &,Buffer *)>;

}

#endif