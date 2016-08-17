#ifndef _HTTP_SERVER_HPP
#define _HTTP_SERVER_HPP

#include "general.hpp"
#include <boost/bind.hpp>
#include <set>

#include "front_conn.hpp"

#include <boost/bimap.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/bimap/multiset_of.hpp>

namespace airobot {

using namespace boost::asio;

class backend_server;

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

    int64_t request_session_id(front_conn_ptr ptr);
    bool set_session_id(front_conn_ptr ptr, uint64_t session_id);
    front_conn_ptr request_connection(uint64_t session_id);

    void set_backend(boost::shared_ptr<backend_server> ptr)
    {
        backend_ = ptr;
    }

    void push_backend(uint64_t site_id, const char* dat, size_t len);
    void show_conns_info(bool verbose);

private:
    io_service io_service_;

    ip::tcp::endpoint ep_;
    ip::tcp::acceptor acceptor_;

    void do_accept();
    void accept_handler(const boost::system::error_code& ec, socket_ptr ptr);

    typedef boost::bimap< boost::bimaps::set_of<front_conn_ptr>,
                          boost::bimaps::multiset_of<uint64_t> > front_conn_type;

    front_conn_type front_conns_;
    //std::set<connection_ptr> connections_;
    //std::map<unsigned long long session_id, connection_ptr> connections_;

    boost::shared_ptr<backend_server> backend_;
};

} // END NAMESPACE

#endif //_HTTP_SERVER_HPP
