#ifndef _HTTP_SERVER_HPP
#define _HTTP_SERVER_HPP

#include "general.hpp"
#include "connection.hpp"

#include <boost/bind.hpp>
#include <set>

namespace airobot {

using namespace boost::asio;

class http_server
{
public:
    //不准拷贝、赋值
    http_server(const http_server&) = delete;
    http_server& operator=(const http_server&) = delete;

    /// Construct the server to listen on the specified TCP address and port, and
    /// serve up files from the given directory.
    explicit http_server(const std::string& address, unsigned short port,
                    const std::string& doc_root);

    /// Run the server's io_service loop.
    void run();

private:
    io_service io_service_;

    ip::tcp::endpoint ep_;
    ip::tcp::acceptor acceptor_;

    void do_accept();
    void accept_handler(const boost::system::error_code& ec, socket_ptr ptr);

    std::set<connection_ptr> connections_; // map latter
    //std::map<unsigned long long session_id, connection_ptr> connections_;
};

} // END NAMESPACE

#endif //_HTTP_SERVER_HPP
