## Rabbit proxy server

Author: Kang Lin (kl222@126.com)

- [:cn: Chinese](README_zh_CN.md)

### Introduction

This software implements multiple protocol proxy service functions.

- Featured function: can access the host in the intranet based on the Internet.
  See: https://github.com/KangLin/RabbitRemoteControl/issues/7

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

### Development
#### Dependent
##### Tools
- [x] [Qt](qt.io)
- [x] c compiler
  + gcc
  + MSVC
- [cmake](https://cmake.org/)

##### libraries

- [Option] libdatachannel: https://github.com/paullouisageneau/libdatachannel
