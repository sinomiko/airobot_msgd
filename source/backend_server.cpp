#include "backend_server.hpp"
#include "http_server.hpp"
#include "front_conn.hpp"

namespace airobot {

backend_server::backend_server(const std::string& address, unsigned short port) :
    io_service_(), 
    ep_(ip::tcp::endpoint(ip::address::from_string(address), port)),
    acceptor_(io_service_, ep_),
    backend_conns_(),
    http_(nullptr)
{
    acceptor_.set_option(ip::tcp::acceptor::reuse_address(true));
    acceptor_.listen();

    do_accept();
}

/// Run the server's io_service loop.

void backend_server::run()
{
    io_service_.run();
}

void backend_server::do_accept()
{
    socket_ptr p_sock(new ip::tcp::socket(io_service_));
    acceptor_.async_accept(*p_sock, 
                           boost::bind(&backend_server::accept_handler, this, 
                                       boost::asio::placeholders::error, p_sock));
}

void backend_server::accept_handler(const boost::system::error_code& ec, socket_ptr p_sock)
{
    if (ec)
    {
        std::cerr << "Error found at:" << __LINE__ << endl;
        return;
    }

    cout << "Client Info: " << p_sock->remote_endpoint().address() <<
        p_sock->remote_endpoint().port() << endl;

    backend_conn_ptr new_c = boost::make_shared<backend_conn>(p_sock, *this);
    backend_conns_.push_back(new_c);
    new_c->start();

    // 再次启动接收异步请求
    do_accept();
}

void backend_server::push_front(uint64_t session_id, const char* dat, size_t len)
{
    if (!session_id || !dat || !len)
        return;
    
    front_conn_ptr ptr = http_->request_connection(session_id);
    
    if (ptr == nullptr) 
        return;

    ptr->fill_and_send(dat, len);

    return;
}

backend_conn_ptr backend_server::require_backend_conn(uint64_t site_id)
{
    if (!backend_conns_.size())
        return nullptr;
    
    return backend_conns_[site_id % backend_conns_.size()];
}

}  // END NAMESPACE