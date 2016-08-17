#include "backend_conn.hpp"
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
    //server_.set_session_id(shared_from_this(), (int64_t)-1);
    set_stats(conn_pending);
    p_sock_->close();
}


void backend_conn::read_handler(const boost::system::error_code &ec, size_t bytes_transferred)
{
    if (!ec && bytes_transferred) 
    {
        cout << &(*p_buffer_)[0] << endl;

    }
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
        cerr << "WRITE ERROR FOUND!" << endl;
        set_stats(conn_error);
    }
}

}
