#include "connection.hpp"
#include "reply.hpp"
#include "http_server.hpp"

#include "json11.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;
using namespace boost::gregorian;

namespace airobot {

connection::connection(boost::shared_ptr<ip::tcp::socket> p_sock):
    touch_time_(second_clock::local_time()), 
    p_sock_(p_sock)
{
    p_buffer_ = boost::make_shared<std::vector<char> >(16*1024, 0);
    p_write_  = boost::make_shared<std::vector<char> >(16*1024, 0); 
}

void connection::start()
{
    do_read();
}

void connection::do_read()
{
    p_sock_->async_read_some(buffer(*p_buffer_), 
                             boost::bind(&connection::read_handler, 
                                  this, 
                                  boost::asio::placeholders::error, 
                                  boost::asio::placeholders::bytes_transferred));
}

void connection::do_write()
{

    p_sock_->async_write_some(buffer(*p_write_), 
                         boost::bind(&connection::write_handler,
                                  this, 
                                  boost::asio::placeholders::error, 
                                  boost::asio::placeholders::bytes_transferred));
}

}
