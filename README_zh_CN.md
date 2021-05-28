## 玉兔代理服务器
作者: 康林(kl222@126.com)

- [:us: 英语](README.md)

### 简介

本软件实现多种协议代理服务功能。

- 特色功能：可以基于互连网访问内网中的主机。详见：https://github.com/KangLin/RabbitRemoteControl/issues/7
  
### 支持的协议

- Socks 代理协议
  - [x] SOCKS 协议版本 4: 
    + socks4:
      - https://www.openssh.com/txt/socks4.protocol
      - https://github.com/tsaooo/Socks4-proxy-server/blob/master/NP_Project4_Spec.pdf
      - https://en.wikipedia.org/wiki/SOCKS#SOCKS4
    + [x] socks4a:
      - https://www.openssh.com/txt/socks4a.protocol
      - https://en.wikipedia.org/wiki/SOCKS#SOCKS4a
  - [x] SOCKS 协议版本 5(RFC1928)：http://www.ietf.org/rfc/rfc1928.txt
    + [x] Socks5 用户名/密码验证协议: https://www.ietf.org/rfc/rfc1929.txt
  - [ ] SOCKS 协议版本 6: https://datatracker.ietf.org/doc/draft-olteanu-intarea-socks-6/
- Http 代理协议
  + [ ] [Hypertext Transfer Protocol (HTTP/1.1): Message Syntax and Routing](https://datatracker.ietf.org/doc/html/rfc7230)
  + [ ] Tunneling TCP based protocols through Web proxy servers: https://datatracker.ietf.org/doc/html/draft-luotonen-web-proxy-tunneling-01
  + [ ] [Hypertext Transfer Protocol (HTTP/1.1): Semantics and Content](https://datatracker.ietf.org/doc/html/rfc7231#section-4.3.6)
  
### 参考文档

- socket5的实现--(RFC1928)Socket5协议中文文档: https://www.cnblogs.com/xi-jie/articles/10476545.html
- HTTP协议和SOCKS5协议: https://www.cnblogs.com/yinzhengjie/p/7357860.html
- HTTP 代理原理及实现（一）: https://imququ.com/post/web-proxy.html

### 开发
#### 依赖
##### 工具
- [x] [Qt](qt.io)
- [x] c compiler
  + gcc
  + MSVC
- [cmake](https://cmake.org/)

##### 库

- [可选] libdatachannel: https://github.com/paullouisageneau/libdatachannel
