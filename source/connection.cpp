#include "connection.hpp"
#include "reply.hpp"

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

void connection::stop()
{
    p_sock_->close();
}

void connection::do_read()
{
    p_sock_->async_read_some(buffer(*p_buffer_), 
                             boost::bind(&connection::read_handler, 
                                  shared_from_this(), 
                                  boost::asio::placeholders::error, 
                                  boost::asio::placeholders::bytes_transferred));
}

void connection::do_write()
{

    p_sock_->async_write_some(buffer(*p_write_), 
                         boost::bind(&connection::write_handler,
                                  shared_from_this(), 
                                  boost::asio::placeholders::error, 
                                  boost::asio::placeholders::bytes_transferred));
}

void connection::read_handler(const boost::system::error_code& ec, size_t bytes_transferred)
{
    if (!ec && bytes_transferred) 
    {
        cout << &(*p_buffer_)[0] << endl;

        // read more
        string ret = reply::reply_generate(string(&(*p_buffer_)[0]));
        memcpy(p_write_->data(), ret.c_str(), ret.size()+1);

        do_write();

        do_read();
    }
    else if (ec != boost::asio::error::operation_aborted)
    {
        cerr << "READ ERROR FOUND!" << endl;
        shared_from_this()->stop();
    }

    return;
}

void connection::write_handler(const boost::system::error_code& ec, size_t bytes_transferred)
{
    if (!ec && bytes_transferred) 
    {
        cout << "WRITE OK!" << endl;

        //不会主动调读的
        //do_write();
    }
    else if (ec != boost::asio::error::operation_aborted)
    {
        cerr << "WRITE ERROR FOUND!" << endl;
        shared_from_this()->stop();
    }
}


}
