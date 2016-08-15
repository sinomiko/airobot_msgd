#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include "general.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;
using namespace boost::gregorian;

namespace airobot {

class connection
: public std::enable_shared_from_this<connection> {

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
    void do_wirte();

    boost::shared_ptr<ip::tcp::socket> p_sock_;
    ptime touch_time_;
};

}


#endif //_CONNECTION_H_
