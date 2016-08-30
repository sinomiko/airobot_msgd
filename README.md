# airobot_msgd   

[![Language](https://img.shields.io/badge/Language-GCC-green.svg)](https://gcc.gnu.org/) 
[![Author](https://img.shields.io/badge/author-Nicol%20TAO-blue.svg)](https://taozj.org/) 
[![License](https://img.shields.io/badge/license-BSD-red.svg)](http://yanyiwu.mit-license.org)

## Purpose   
This is a high-performance front message distribute system based on C++11 and boost::asio.

If your production environment sounds like a front-side item used to collect all requests( in json format), and then distributes them to backend workers, after calc and process then reponse the answer backed to the hold connection, this system may great suit you. 
Actually many message queue( or middleware) systems support REQUEST-RESPONSE with proxy can works well, but it also need your server-side and client-side to be reconstructed, which seems impossible sometimes. This library can works well with your already system in production, it distribute your front requests to back worker(s) almost in fair, and can atomatic destroy bad connections to avoid resource leakage. With boost::asio, the famous asynchronous program method can guarantee the performance, and thread pool will be added later.

## Quick Start   
(1) type make and start this server:   
```bash
user@gentoo airobot_msgd % ./Debug/airobot_msgd
infomanage.cpp:62[@boost_log_init] BOOST LOG V2 Initlized OK!
infomain.cpp:30[@main] System hardware_concurrency: 4
infomain.cpp:34[@main] Server Runing At:0.0.0.0:5598
infomain.cpp:35[@main] DocumentRoot:./
infomain.cpp:40[@main] Backend Server Runing At:0.0.0.0:5590
ThreadID: 7f1ae2c35700
ThreadID: 7f1ae2434700
```

(2) on another terminal, start backend server:   
```bash
user@gentoo ~ % telnet 127.0.0.1 5590
Trying 127.0.0.1...
Connected to 127.0.0.1.
Escape character is '^]'.

```

(3) Siege it front:   
```bash
user@gentoo ~ % siege -c30  'http://192.168.1.221:5598/bb POST { "site_id":101964,"session_id":105408569209258625 } '
```

The transform rate like:
```bash
Lifting the server siege...      done.

Transactions:                674 hits
Availability:             100.00 %
Elapsed time:              10.71 secs
Data transferred:           0.00 MB
Response time:              0.00 secs
Transaction rate:          62.93 trans/sec
Throughput:             0.00 MB/sec
Concurrency:                0.11
Successful transactions:         674
Failed transactions:               0
Longest transaction:            0.01
Shortest transaction:           0.00

```

If interested with it, just try it!