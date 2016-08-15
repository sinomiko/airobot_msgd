#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include "general.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;
using namespace boost::gregorian;

namespace airobot {

class connection
{

public:
    connection(const connection&) = delete;
    connection& operator=(const connection&) = delete;

    /// Construct a connection with the given socket.
    explicit connection(boost::shared_ptr<ip::tcp::socket> p_sock);

    void start();
    void stop();

private:
    // 异步IO
    void do_read();
    void do_write();

    void read_handler(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void write_handler(const boost::system::error_code &ec, std::size_t bytes_transferred); 

    boost::shared_ptr<ip::tcp::socket> p_sock_;
    ptime touch_time_;
    boost::shared_ptr<std::vector<char> > p_buffer_;
};

}


#endif //_CONNECTION_H_
