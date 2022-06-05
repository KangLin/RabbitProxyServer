## Rabbit proxy server

Author: Kang Lin (kl222@126.com)

- Project
  + Home: [https://kanglin.github.io/RabbitProxyServer](https://kanglin.github.io/RabbitProxyServer/)
  + Project main repository: [https://github.com/KangLin/RabbitProxyServer](https://github.com/KangLin/RabbitProxyServer)
    - Mirror:
      + sourceforge: https://sourceforge.net/projects/rabbitproxyserver/
      + gitlab: https://gitlab.com/kl222/RabbitProxyServer
  + [Mailing list](https://sourceforge.net/p/rabbitproxyserver/mailman/): <rabbitproxyserver-discuss@lists.sourceforge.net>

- Language:
  + [:cn: Chinese](README_zh_CN.md)
  
- Build status
  - [![Build status](https://ci.appveyor.com/api/projects/status/id993rbqmx147cqw?svg=true)](https://ci.appveyor.com/project/KangLin/rabbitproxyserver)
  - [![CMake ubuntu](https://github.com/KangLin/RabbitProxyServer/actions/workflows/cmake_ubuntu.yml/badge.svg)](https://github.com/KangLin/RabbitProxyServer/actions/workflows/cmake_ubuntu.yml)
  - [![CMake msvc](https://github.com/KangLin/RabbitProxyServer/actions/workflows/msvc.yml/badge.svg)](https://github.com/KangLin/RabbitProxyServer/actions/workflows/msvc.yml)

### Introduction

Due to work reasons, it is often necessary to remotely control a host in another intranet from one intranet. For example, in the following figure: Computer 1 in network 1 accesses the server or computer 2 in network 2.

![Network Topology](Documents/Image/network_en.svg)

No relevant open source software was found on the Internet.
So I wrote [Rabbit Remote Control](https://github.com/KangLin/RabbitRemoteControl). 
In the process of writing, I encountered a problem of mutual access between two intranets.
For details, please see: https://github.com/KangLin/RabbitRemoteControl/issues/7.

Searching for related proxy software on the Internet, there are only proxy servers with public IPs, and no proxy servers without public IPs. So I wrote this software,
In order to solve the problem of mutual access between two internal networks without public IP. And this software implements [multiple protocols](#Supported-protocols) proxy service function.

### Donation
If this software is useful to you, or you like it, please donate and support the author. Thank you!

[![donation](https://github.com/KangLin/RabbitCommon/raw/master/Src/Resource/image/Contribute.png "donation")](https://github.com/KangLin/RabbitCommon/raw/master/Src/Resource/image/Contribute.png "donation")

If it cannot be displayed, please open:
https://gitee.com/kl222/RabbitCommon/raw/master/Src/Resource/image/Contribute.png

### Supported protocols

- Socks protocol
  - [x] SOCKS Protocol Version 4: 
    + socks4:
      - https://www.openssh.com/txt/socks4.protocol
      - https://github.com/tsaooo/Socks4-proxy-server/blob/master/NP_Project4_Spec.pdf    
      - https://en.wikipedia.org/wiki/SOCKS#SOCKS4
    + [x] socks4a:
      - https://www.openssh.com/txt/socks4a.protocol
      - https://en.wikipedia.org/wiki/SOCKS#SOCKS4a
  - [x] SOCKS Protocol Version 5 (RFC1928)：http://www.ietf.org/rfc/rfc1928.txt
    + [x] Username/Password Authentication for SOCKS V5: https://www.ietf.org/rfc/rfc1929.txt
  - [ ] SOCKS Protocol Version 6: https://datatracker.ietf.org/doc/draft-olteanu-intarea-socks-6/
- Http protocols
  + [ ] [Hypertext Transfer Protocol (HTTP/1.1): Message Syntax and Routing](https://datatracker.ietf.org/doc/html/rfc7230)
  + [ ] Tunneling TCP based protocols through Web proxy servers: https://datatracker.ietf.org/doc/html/draft-luotonen-web-proxy-tunneling-01
  + [ ] [Hypertext Transfer Protocol (HTTP/1.1): Semantics and Content](https://datatracker.ietf.org/doc/html/rfc7231#section-4.3.6)
- Custom protocol
    + [x] Custom protocol for ICE：[Src/PeerConnecterIceClient.h](Src/PeerConnecterIceClient.h#L63)
    
### Development
#### Dependent
##### Tools
- [x] [Qt](qt.io)
- [x] c compiler
  + gcc
  + MSVC
- [cmake](https://cmake.org/)

##### libraries

- [Optional] libdatachannel: https://github.com/paullouisageneau/libdatachannel
