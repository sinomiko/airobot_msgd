#include <iostream>

#include "http_server.hpp"
#include "reply.hpp"

namespace airobot {

std::string reply::fixed_reply_error;
std::string reply::fixed_reply_ok;


http_server::http_server(const std::string& address, unsigned short port,
                    const std::string& doc_root) :
    io_service_(), 
    ep_(ip::tcp::endpoint(ip::address::from_string(address), port)),
    acceptor_(io_service_, ep_)
{
    acceptor_.set_option(ip::tcp::acceptor::reuse_address(true));
    acceptor_.listen();

    do_accept();
}

/// Run the server's io_service loop.

void http_server::run()
{
    string err =  "<html>"
                  "<head><title>Internal Server Error</title></head>"
                  "<body><h1>500 Internal Server Error</h1></body>"
                  "</html>";
    reply::fixed_reply_error = reply::reply_generate(err, http_proto::status::internal_server_error); 
    reply::fixed_reply_ok = reply::reply_generate("{}", http_proto::status::ok);
    io_service_.run();
}

void http_server::do_accept()
{
    socket_ptr p_sock(new ip::tcp::socket(io_service_));
    acceptor_.async_accept(*p_sock, 
                           boost::bind(&http_server::accept_handler, this, 
                                       boost::asio::placeholders::error, p_sock));
}

void http_server::accept_handler(const boost::system::error_code& ec, socket_ptr p_sock)
{
    if (ec)
    {
        std::cerr << "Error found at:" << __LINE__ << endl;
        return;
    }

    cout << "Client Info: " << p_sock->remote_endpoint().address() << "/" <<
        p_sock->remote_endpoint().port() << endl;

    connection_ptr new_c = boost::make_shared<connection>(p_sock);
    connections_.insert(new_c);
    new_c->start();

    // 再次启动接收异步请求
    do_accept();
}

} // END NAMESPACE 


