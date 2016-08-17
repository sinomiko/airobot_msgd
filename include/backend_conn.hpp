#ifndef _BACKEND_CONN_H_
#define _BACKEND_CONN_H_

#include "general.hpp"
#include "connection.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;
using namespace boost::gregorian;

#include <boost/enable_shared_from_this.hpp>

namespace airobot {
class backend_server;

class backend_conn : public connection, public boost::enable_shared_from_this<backend_conn>
{
public:
    backend_conn(const backend_conn&) = delete;
    backend_conn& operator=(const backend_conn&) = delete;

    /// Construct a connection with the given socket.
    backend_conn(boost::shared_ptr<ip::tcp::socket> p_sock, backend_server& server);

    virtual void stop() override;
    virtual void read_handler(const boost::system::error_code &ec, std::size_t bytes_transferred) override; 
    virtual void write_handler(const boost::system::error_code &ec, std::size_t bytes_transferred) override; 

private:
    backend_server& server_;
};

typedef boost::shared_ptr<backend_conn> backend_conn_ptr;

}


#endif //_BACKEND_CONN_H_
