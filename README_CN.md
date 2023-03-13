## Bench4Cpp
[简体中文](README_CN.md) | [English](./README.md)

WebBench4Cpp 是一个用于测试web服务器性能的工具。 它是用C++编写的，基于[Muduo](https://github.com/chenshuo/muduo)网络库改写而来。


### Usage
```
Usage: webbench [option]... URL
  -t|--time <sec>           运行测试的时间，单位秒，默认30秒。
  
  -v|--http version <1.0|1.1|2.0>
                            使用的HTTP协议版本，1.0或1.1或2.0。
  -c|--clients <n>          同时运行的客户端数量，默认256。
  -m|--method <method>      HTTP方法，默认GET。
  -u|--url  <url>           HTTP url。url的路径不能为空。
```

### Example
```
./WebBench4Cpp -c 4096 -t 60 -u http://www.baidu.com/    -v 1.1 -m GET 
# 4096 clients, 60 seconds,   http version 1.1, GET method
```


### Build
```
使用xmake工具构建 (https://xmake.io/#/zh-cn/)
xmake build 
xmake run bench  -c 1024 -t 60 -v 1.1 -m GET -u http://www.baidu.com/
```



### TODO
- [ ] 添加更多的HTTP方法
- [ ] 支持https
- [ ] 格式化输出
- [ ] 添加更多的选项，比如从文件中加载http头，等等。
- [ ] 添加更多的输出信息，比如平均响应时间，等等。