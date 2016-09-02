#ifndef _BACKEND_SERVER_HPP
#define _BACKEND_SERVER_HPP

#include "general.hpp"
#include "backend_conn.hpp"

#include <boost/bind.hpp>
#include <boost/asio/deadline_timer.hpp>

namespace airobot {

class http_server;

static constexpr int BACKEND_CHECK_INTERVEL = 5; //5s

class backend_server {
public:
    //不准拷贝、赋值
    backend_server(const backend_server&) = delete;
    backend_server& operator=(const backend_server&) = delete;

    /// Construct the server to listen on the specified TCP address and port, and
    /// serve up files from the given directory.
    explicit backend_server(const objects* daemons, 
                            const std::string& address, unsigned short port);

    /// Run the server's io_service loop.
    void run();

    void push_front(uint64_t session_id, const char* dat, size_t len);
    backend_conn_ptr require_backend_conn(uint64_t site_id);

    void show_conns_info(bool verbose);
    void on_check_timeout(const boost::system::error_code& e);

private:
    io_service io_service_;

    ip::tcp::endpoint ep_;
    ip::tcp::acceptor acceptor_;

    void do_accept();
    void accept_handler(const boost::system::error_code& ec, socket_ptr ptr);

    std::vector<backend_conn_ptr> backend_conns_; // map latter

    boost::asio::basic_deadline_timer< boost::posix_time::ptime > check_timer_;

    http_server* http_; //fast access
    const objects* daemons_;  
};

} // END NAMESPACE

#endif //_BACKEND_SERVER_HPP
