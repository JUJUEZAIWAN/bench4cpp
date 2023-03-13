
#ifndef REQUEST_H
#define REQUEST_H

#include <string>
#include <unordered_map>

#include <netinet/in.h>

using std::string;

class Request
{
public:
    Request(const string &url,const string &method="GET",const string &version="1.1");
    ~Request();
    void setRequest(const string &request);
    string getRequest() const;

    void loadRequestFromFile(const string &filename);

    void defaultRequest();
    struct sockaddr_in getServerAddr() const;

    void addHeader(const string &header);
    void addHeader(std::unordered_map<string, string> &headers);
    void addHeader(const string &key, const string &value);
    void addBody(const string &body);


private:

    void parseUrl();


private:
    string request_;
    struct sockaddr_in serverAddr_;
    string url_;
    string method_;
    string version_;
    string host_;
    string path_;
    int port_;



};




#endif

