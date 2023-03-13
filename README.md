# Bench4Cpp

[简体中文](README_CN.md) | [English](./README.md)

Bench4Cpp is a tool for benchmarking web servers. It is written in C++ and is based on the [Muduo](https://github.com/chenshuo/muduo) network library.

### Usage

```
Usage: ./webbench [option]... 
  -t|--time <sec>          Run benchmark for <sec> seconds. Default 30.
  -v|--http version <1.0|1.1|2.0>
                           Use HTTP protocol version 1.0 or 1.1 or 2.0.
  -c|--clients <n>         Run <n> HTTP clients at once. Default 256.
  -m|--method <method>     HTTP method. Default GET.
  -u|--url <url>           HTTP url. the path of the url must not be empty.
```

### Example

```
./bench -c 4096 -t 60 -u http://www.baidu.com/   -v 1.1 -m GET 
# 4096 clients, 60 seconds, http version 1.1, GET method
```

### Build

```
use xmake build tool (https://xmake.io/#/zh-cn/)
xmake build
xmake run bench  -c 1024 -t 60 -v 1.1 -m GET -u http://www.baidu.com/
```

### TODO

- [ ] Add more HTTP methods
- [ ] to support https
- [ ] Format the output
- [ ] Add more options,such as load http header from file,etc.
- [ ] Add more information in the output,such as the average response time,etc.
