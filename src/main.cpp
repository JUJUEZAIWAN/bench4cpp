#include "Client.h"
#include "EventLoop.h"
#include "cmdline.h" // parse command line
#include "Request.h"

#include <iostream>
#include <string>
#include <vector>
#include <atomic>
#include <thread>

#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

using std::string;
using namespace stone;
using namespace cmdpaser;

class Http
{

public:
    Http(EventLoop *loop, int numsocket, int numthread, struct sockaddr_in serverAddr, int seconds)
        : client_(loop, numsocket, serverAddr), numConnSocket_(0),
          send_len_(0), recv_len_(0), send_num_(0), recv_num_(0), seconds_(seconds)
    {
        client_.setConnectionCallback(
            [this](const TcpConnectionPtr &conn)
            {
                onConnection(conn);
            });
        client_.setMessageCallback(
            [this](const TcpConnectionPtr &conn, Buffer *buf)
            {
                onMessage(conn, buf);
            });
        client_.setThreadNum(numthread);
    }
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            numConnSocket_++;
            conn->send(requestData_);
            send_len_ += requestData_.size();
            send_num_++;
        }
        else
        {
            conn->forceClose();
        }
    }

    void onMessage(const TcpConnectionPtr &conn, Buffer *buf)
    {
        recv_len_ += buf->readableBytes();
        buf->retrieveAll();
        recv_num_++;
        conn->send(requestData_);
        send_len_ += requestData_.size();
        send_num_++;
    }

    void start()
    {
        client_.start();
        std::thread t([this]()
                      {
                        
                            auto now = std::chrono::system_clock::now();
                            time_t tt = std::chrono::system_clock::to_time_t(now);
                            std::cout << "bench begin at   :" << ctime(&tt) << std::endl;
                            
                            std::this_thread::sleep_for(std::chrono::seconds(seconds_));
                            
                            client_.quit();
                            now = std::chrono::system_clock::now();
                            tt = std::chrono::system_clock::to_time_t(now);
                            std::cout << "bench end at   :" << ctime(&tt) << std::endl;
                            
                        });
        t.detach();
    }

    void setRequestData(const string &requestData)
    {
        requestData_ = requestData;
    }

public:
    void showResult()
    {
        std::cout << "success connection :" << numConnSocket_ << std::endl;
        std::cout << "send_len_ :" << (long double)(send_len_) / (1024 * 1024) <<"(MB)"<< std::endl;
        std::cout << "recv_len_:" << (long double)(recv_len_) / (1024 * 1024)  <<"(MB)"<< std::endl;
        std::cout << "send_num_:" << send_num_ << std::endl;
        std::cout << "recv_num_:" << recv_num_ << std::endl;
        std::cout << "send speed:" << (long double)(send_len_) / (1024 * 1024) / seconds_ << " (MB/s)" << std::endl;
        std::cout << "recv speed:" << (long double)(recv_len_) / (1024 * 1024) / seconds_ << " (MB/s)" << std::endl;
        std::cout << "send frequency :" << (long double)(send_num_) / seconds_ << " times/s" << std::endl;
        std::cout << "recv frequency :" << (long double)(recv_num_) / seconds_ << " times/s" << std::endl;
    }

private:
    string requestData_;
    int numConnSocket_;
    Client client_;
    std::atomic<long long> send_len_;
    std::atomic<long long> recv_len_;
    std::atomic<long long> send_num_;
    std::atomic<long long> recv_num_;
    int seconds_;
};

int main(int argc, char **argv)
{

    Parser p;
    p.add<int>("client", 'c', "client numbers", 256, cmdpaser::rangeof<int>(1, 65535));
    p.add<int>("time", 't', "run time(seconds)", 30, cmdpaser::rangeof<int>(1, 65535));
    p.add<string>("url", 'u', "url", [](const string &url)
                  { return url.size() < 1500; });
    p.add<string>("version", 'v', "http protocol version", "1.1", cmdpaser::oneof<string>({"1.0", "1.1", "2.0"}));
    p.add<string>("method", 'm', "method", "GET", cmdpaser::oneof<string>({"GET", "POST", "PUT", "DELETE", "HEAD", "OPTIONS", "PATCH"}));
    p.parse(argc, argv);

    auto clients = p.get<int>("client");
    auto run_time = p.get<int>("time");
    auto url = p.get<string>("url");
    auto protocol = p.get<string>("version");
    auto method_s = p.get<string>("method");

    Request request(url, method_s, protocol);
    request.defaultRequest();
    string requestData = request.getRequest();

    EventLoop loop;

    struct sockaddr_in serverAddr = request.getServerAddr();
    int core_num = sysconf(_SC_NPROCESSORS_ONLN);

    Http http(&loop, clients, core_num, serverAddr, run_time);
    http.setRequestData(requestData);
    http.start();
    loop.loop();
    http.showResult();
}