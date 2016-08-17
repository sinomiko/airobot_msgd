#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include "general.hpp"
#include "http_proto.hpp"
#include "http_parser.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;
using namespace boost::gregorian;

namespace airobot {

class http_server;

class connection
{

public:
    connection(const connection&) = delete;
    connection& operator=(const connection&) = delete;

    /// Construct a connection with the given socket.
    explicit connection(boost::shared_ptr<ip::tcp::socket> p_sock);

    void start();
    virtual void stop() = 0;

protected:
    // 异步IO
    void do_read();
    void do_write();

    virtual void read_handler(const boost::system::error_code& ec, std::size_t bytes_transferred) = 0;
    virtual void write_handler(const boost::system::error_code &ec, std::size_t bytes_transferred) = 0; 

    boost::shared_ptr<ip::tcp::socket> p_sock_;
    ptime touch_time_;
    boost::shared_ptr<std::vector<char> > p_buffer_;
    boost::shared_ptr<std::vector<char> > p_write_;

    virtual ~connection() {}
  
};

}


#endif //_CONNECTION_H_
