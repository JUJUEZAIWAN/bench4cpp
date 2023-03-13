
#include "Request.h"

#include <iostream>
#include <fstream>

#include <arpa/inet.h>
#include <netdb.h>
#include <cassert>

Request::Request(const string &url, const string &method, const string &version) 
    : url_(url), method_(method), version_(version)
{

    parseUrl();
}

Request::~Request()
{
    // nothing
}

void Request::setRequest(const string &request)
{
    request_ = request;
}

string Request::getRequest() const
{
    return request_;
}

void Request::loadRequestFromFile(const string &filename)
{
    std::fstream file(filename, std::ios::in);
    if (!file.is_open())
    {
        std::cout << "file open failed" << std::endl;
        return;
    }
    std::string line;
    while (std::getline(file, line))
    {
        request_ += line;
        if (line[line.size() - 2] != '\r' && line[line.size() - 1] != '\n')
        {
            request_ += "\r\n";
        }
    }
    request_ += "\r\n\r\n";
}

 
struct sockaddr_in Request::getServerAddr() const
{
    return serverAddr_;
}

void Request::parseUrl()
{
    if (url_.find("http://") != 0)
    {
        std::cout << "url is not http" << std::endl;
        abort();
    }
    string::size_type pos = 7;
    string::size_type pos2 = url_.find('/', pos);

    if (pos2 == string::npos)
    {
        host_ = url_.substr(pos);
        path_ = "/";
    }
    else
    {
        host_ = url_.substr(pos, pos2 - pos);
        path_ = url_.substr(pos2);
    }

    pos = host_.find(':');
    if (pos == string::npos)
    {
        port_ = 80;
    }
    else
    {
        port_ = atoi(host_.substr(pos + 1).c_str());
        host_ = host_.substr(0, pos);
    }

    serverAddr_.sin_family = AF_INET;
    serverAddr_.sin_port = htons(port_);

    struct in_addr addr;
    if (inet_aton(host_.c_str(), &addr))
    {
        serverAddr_.sin_addr = addr;
    }
    else
    {
        struct hostent *host = gethostbyname(host_.c_str());
        if (host == nullptr)
        {
            std::cout << "gethostbyname failed" << std::endl;
            abort();
        }
        serverAddr_.sin_addr = *(struct in_addr *)host->h_addr;
    }
}

void Request::defaultRequest()
{
    string tmp ="GET / HTTP/1.1\r\nHost: www.baidu.com\r\n\r\n";
    // request_  ="GET / HTTP/1.1\r\nHost: www.baidu.com\r\n\r\n";

    request_ =  method_+" " + path_ + " HTTP/"+ version_+"\r\n";
    request_ += "Host: " + host_ + "\r\n";
    request_ += "\r\n";
    assert(request_ == tmp);

}

void Request::addHeader(const string &header)
{
    request_ += header;
    if (header[header.size() - 2] != '\r' && header[header.size() - 1] != '\n')
    {
        request_ += "\r\n";
    }
}
void Request::addHeader(std::unordered_map<string, string> &headers)
{
    for (auto &header : headers)
    {
        request_ += header.first + ": " + header.second + "\r\n";
    }
}
void Request::addHeader(const string &key, const string &value)
{
    request_ += key + ": " + value + "\r\n";
}
void Request::addBody(const string &body)
{
    request_ += "\r\n" + body;
}