#include "backend_server.hpp"
#include "http_server.hpp"
#include "front_conn.hpp"
#include "http_proto.hpp"

#include <boost/format.hpp>

namespace airobot {

static boost::condition_variable conn_notify;
static boost::mutex conn_notify_mutex;

backend_server::backend_server(const std::string& address, unsigned short port) :
    io_service_(),
    ep_(ip::tcp::endpoint(ip::address::from_string(address), port)),
    acceptor_(io_service_, ep_),
    backend_conns_(),
    http_(nullptr),
    check_timer_(io_service_)
{
    acceptor_.set_option(ip::tcp::acceptor::reuse_address(true));
    acceptor_.listen();

    check_timer_.expires_from_now(boost::posix_time::seconds(BACKEND_CHECK_INTERVEL)); 
    check_timer_.async_wait(boost::bind(&backend_server::on_check_timeout,
                                        this, boost::asio::placeholders::error));

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
        BOOST_LOG_T(error) << "Error during accept";
        return;
    }

    BOOST_LOG_T(debug)  << "Client Info: " << p_sock->remote_endpoint().address() << "/"
        << p_sock->remote_endpoint().port();

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

    ptr->fill_for_http(dat, len, http_proto::status::ok);
    ptr->do_write();

    return;
}

backend_conn_ptr backend_server::require_backend_conn(uint64_t site_id)
{
    size_t index = 0;
    size_t flag = 0;

    if (!backend_conns_.size())
        return nullptr;

    index = site_id % backend_conns_.size();
    if (backend_conns_[index]->get_stats() == conn_working)
        return backend_conns_[index];

    // 如果是不可工作的，轮训
    flag = index;
    do{
        index = (index+1)%backend_conns_.size();
        if (backend_conns_[index]->get_stats() == conn_working)
            return backend_conns_[index];
    }while(index != flag);

    return nullptr;
}


void backend_server::show_conns_info(bool verbose)
{
    size_t total_cnt = 0, err_cnt = 0, work_cnt = 0, pend_cnt = 0;

    for (auto &item: backend_conns_)
    {
        if (verbose) 
            cout << boost::format("backend_conn[%d], touched:%s, status: " )
                % total_cnt % to_simple_string(item->touch_time_); 

        if (item->get_stats() == conn_working)
        { work_cnt++; if (verbose)cout << "working" << endl; }
        if (item->get_stats() == conn_pending)
        { pend_cnt++; if (verbose) cout << "pending" << endl; }
        if (item->get_stats() == conn_error)
        { err_cnt++;  if (verbose) cout << "error" << endl; }

        total_cnt ++;
    }

    cout << boost::format("[BACKEND_SERVER] total:%d, working:%d, pending:%d, error:%d ")
        % total_cnt % work_cnt % pend_cnt % err_cnt << endl;

    return;
}


void backend_server::on_check_timeout(const boost::system::error_code& e)
{
    if (e != boost::asio::error::operation_aborted)
    {
        // 检查后端连接
        if (backend_conns_.empty())
            goto return_entry;

        // std::vector迭代修改是不允许的，同时大多数情况应该都是正常的，
        // 首先遍历如果没有错误的，就快速放回
        // 否则拷贝一份新的正常的，再将原来的拷贝过去
        bool all_ok = true;
        for (auto& item: backend_conns_)
        {
            if (item->get_stats() == conn_error)
            {
                all_ok = false;
                break;
            }
        }

        if (all_ok) goto return_entry;
        
        std::vector<backend_conn_ptr> new_store;
        for (auto& item: backend_conns_)
        {
            if (item->get_stats() == conn_error)
            {
                /**/;
            }
            else
            {
                new_store.push_back(item);
            }
        }

        // delete it!
        backend_conns_.clear();
        backend_conns_ = std::move(new_store);
    }

return_entry:
    // 再次绑定
    check_timer_.expires_from_now(boost::posix_time::seconds(BACKEND_CHECK_INTERVEL));
    check_timer_.async_wait(boost::bind(&backend_server::on_check_timeout,
                                        this, boost::asio::placeholders::error));
}



}  // END NAMESPACE
