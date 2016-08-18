airobot_msgd

This is a high-performance front message distribute system based on C++11 and boost::asio.

If your production environment sounds like a front-side item used to collect all requests( in json format), and then distributes them to backend workers, after calc and process then reponse the answer backed to the hold connection, this system may great suit you. 
Actually many message queue( or middleware) systems support REQUEST-RESPONSE with proxy can works well, but it also need your server-side and client-side to be reconstructed, which seems impossible sometimes. This library can works well with your already system in production, it distribute your front requests to back worker(s) almost in fair, and can atomatic destroy bad connections to avoid resource leakage. With boost::asio, the famous asynchronous program method can guarantee the performance, and thread pool will be added later.

If interested with it, just try it!