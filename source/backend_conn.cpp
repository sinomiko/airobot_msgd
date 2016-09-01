#include "backend_conn.hpp"
#include "backend_server.hpp"
#include "json11.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;
using namespace boost::gregorian;

namespace airobot {

backend_conn::backend_conn(boost::shared_ptr<ip::tcp::socket> p_sock,
                       backend_server& server):
    connection(p_sock),
    server_(server)
{
    p_buffer_ = boost::make_shared<std::vector<char> >(16*1024, 0);
    p_write_  = boost::make_shared<std::vector<char> >(16*1024, 0); 
}

void backend_conn::stop()
{
    set_stats(conn_pending);
}


void backend_conn::read_handler(const boost::system::error_code &ec, size_t bytes_transferred)
{
    if (!ec && bytes_transferred) 
    {
        //cout << p_buffer_->data() << endl;
        string json_err;
        auto json_parsed = json11::Json::parse(p_buffer_->data(), json_err);
        uint64_t session_id = static_cast<uint64_t>(json_parsed["session_id"].uint64_value());
        uint64_t site_id = static_cast<uint64_t>(json_parsed["site_id"].uint64_value());

        if (session_id == 0 || site_id == 0) 
            goto ok_no_return;
        

        // TODO: 转发到前台
        server_.push_front(session_id, p_buffer_->data(), strlen(p_buffer_->data())+1);
    }
    else if (ec != boost::asio::error::operation_aborted)
    {
        BOOST_LOG_T(error) <<  "READ ERROR FOUND!";
        p_sock_->close();
        set_stats(conn_error);

        // maybe we can delete ourself from connection container
        // directly here.
    }

ok_no_return:

    do_read();
    return;
}

void backend_conn::write_handler(const boost::system::error_code& ec, size_t bytes_transferred)
{
    if (!ec && bytes_transferred) 
    {
        //cout << "WRITE OK!" << endl;

        //不会主动调读的
        //do_write();
    }
    else if (ec != boost::asio::error::operation_aborted)
    {
        BOOST_LOG_T(error) << "WRITE ERROR FOUND!";
        p_sock_->close();
        set_stats(conn_error);
    }
}

}
