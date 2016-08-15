#include "connection.hpp"

namespace airobot {

connection::connection(boost::shared_ptr<ip::tcp::socket> p_sock):
    touch_time_(second_clock::local_time()), 
    p_sock_(p_sock)
{
    p_buffer_ = boost::make_shared<std::vector<char> >(16*1024, 0);
}

void connection::start()
{
    do_read();
}

void connection::stop()
{
    p_sock_->close();
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

}

void connection::read_handler(const boost::system::error_code& ec, size_t bytes_transferred)
{
    if (!ec && bytes_transferred) 
    {
        cout << &(*p_buffer_)[0] << endl;

        // read more
        do_read();
    }
    else
        cerr << "ERROR" << endl;

    return;
}

void connection::write_handler(const boost::system::error_code& ec, size_t bytes_transferred)
{

}


}
