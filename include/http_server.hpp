#ifndef _HTTP_SERVER_HPP
#define _HTTP_SERVER_HPP

#include "general.hpp"
#include <boost/bind.hpp>
#include <set>
#include <mutex>
#include <atomic>

#include "front_conn.hpp"

#include <boost/bimap.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/bimap/multiset_of.hpp>

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

namespace airobot {

using namespace boost::asio;

static constexpr int FRONT_EXPIRED_INTERVEL = 30*60; //30min

class backend_server;

class http_server
{
public:
    //不准拷贝、赋值
    http_server(const http_server&) = delete;
    http_server& operator=(const http_server&) = delete;

    /// Construct the server to listen on the specified TCP address and port, and
    /// serve up files from the given directory.
    explicit http_server(const objects* daemons, const std::string& address, unsigned short port,
                    const std::string& doc_root, size_t c_cz);

    /// Run the server's io_service loop.
    void run();

    int64_t request_session_id(front_conn_ptr ptr);
    bool set_session_id(front_conn_ptr ptr, uint64_t session_id);
    front_conn_ptr request_connection(uint64_t session_id);

    class co_worker* get_co_worker() { return daemons_->co_worker_; }

    void push_backend(uint64_t site_id, const char* dat, size_t len);
    void show_conns_info(bool verbose);

    static boost::condition_variable_any conn_notify;
    static boost::mutex conn_notify_mutex;

private:
    friend class front_conn;

    io_service io_service_;

    // 侦听地址信息
    ip::tcp::endpoint ep_;
    ip::tcp::acceptor acceptor_;

    size_t concurr_sz_;

    void do_accept();
    void accept_handler(const boost::system::error_code& ec, socket_ptr ptr);

    typedef boost::bimap< boost::bimaps::set_of<front_conn_ptr>,
                          boost::bimaps::multiset_of<uint64_t> > front_conn_type;

    friend void manage_thread(const objects* daemons);

    std::mutex      front_conns_mutex_;
    front_conn_type front_conns_;

    // 记录front_conns_中的连接数目，便于控制最大服务量
    // 下面这些数据结构同样被front_conns_mutex_保护
    unsigned long long max_serve_conns_cnt_;
    std::atomic_ullong current_conns_cnt_;  

    // 使用limit获得当前进程最大句柄的硬限制
    bool set_max_serve_conns_cnt(unsigned long long cnt);

    // 每次需要删除的conn都放到这个set中，避免manage线程
    // 每次持锁遍历整个连接容器，提高效率。 使用set是为了防止多次插入
    std::set<front_conn_ptr> pending_to_remove_;
    void push_to_remove(front_conn_ptr ptr)
    {
        assert( pending_to_remove_.find(ptr) == pending_to_remove_.end());
        pending_to_remove_.insert(ptr);
    }

    //std::set<connection_ptr> connections_;
    //std::map<unsigned long long session_id, connection_ptr> connections_;

    backend_server* backend_; //fast access
    const objects* daemons_;  
};

} // END NAMESPACE

#endif //_HTTP_SERVER_HPP
