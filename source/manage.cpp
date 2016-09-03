#include "general.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;
using namespace boost::gregorian;

#include <boost/thread.hpp>

#include "http_server.hpp"
#include "backend_server.hpp"

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>

#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes/timer.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/support/date_time.hpp>


/**
 * 包含管理维护线程的相关工作
 */

namespace airobot {


namespace blog_sink = boost::log::sinks;
namespace blog_expr = boost::log::expressions;
namespace blog_keyw = boost::log::keywords;
namespace blog_attr = boost::log::attributes;


void boost_log_init(const string filename_prefix)
{
    boost::log::add_common_attributes();
    //boost::log::core::get()->add_global_attribute("Scope",  blog_attr::named_scope());
    boost::log::core::get()->add_global_attribute("Uptime", blog_attr::timer());

    boost::log::add_file_log(
        blog_keyw::file_name = filename_prefix+"_%N.log",
        blog_keyw::time_based_rotation = 
                blog_sink::file::rotation_at_time_point(0, 0, 0),
        blog_keyw::open_mode = std::ios_base::app,
        blog_keyw::format = blog_expr::stream
           // << std::setw(7) << std::setfill(' ') << blog_expr::attr< unsigned int >("LineID") << std::setfill(' ') << " | "
            << "["   << blog_expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d, %H:%M:%S.%f")
            << "] [" << blog_expr::format_date_time< blog_attr::timer::value_type >("Uptime", "%O:%M:%S")
           // << "] [" << blog_expr::format_named_scope("Scope", blog_keyw::format = "%n (%F:%l)")
            << "] <"  << boost::log::trivial::severity << "> "
            << blog_expr::message,
        blog_keyw::auto_flush = true
        );

    // trace debug info warning error fatal
    boost::log::core::get()->set_filter (
        boost::log::trivial::severity >= boost::log::trivial::trace);

    BOOST_LOG_T(info) << "BOOST LOG V2 Initlized OK!";

    return;
}

extern objects all_daemons;

void manage_thread(const objects* daemons)
{

    cerr<< "Manage ThreadID: " << boost::this_thread::get_id() << endl;
    http_server* p_srv = daemons->http_server_;
    backend_server *p_backend_srv = daemons->backend_server_;

    for (;;)
    {
        boost::unique_lock<boost::mutex> notify_lock(p_srv->conn_notify_mutex); 
        if(p_srv->conn_notify.timed_wait(notify_lock, boost::posix_time::seconds(45)))
        {
            assert(notify_lock.owns_lock());
            if(p_srv->pending_to_remove_.empty())   
                continue;

            // 遍历，剔除失败的连接
            // 删除的时候，还是需要持有锁，因为新建连接的时候会创建插入元素，如果
            // 后面交换的话，会导致数据缺失
            // 这里算是个性能弱势点
            boost::lock_guard<boost::mutex> mutex_lock(p_srv->front_conns_mutex_);

            http_server::front_conn_type::left_map &view = p_srv->front_conns_.left;

            BOOST_LOG_T(info) << "Original connection: " << p_srv->front_conns_.size()
                                << ", trimed connection: " << p_srv->pending_to_remove_.size();

            // pending_to_remove可能在conn_stat以及co_worker timing
            // 两个地方同时被登记删除
            for (auto &item: p_srv->pending_to_remove_) 
            {
                if(view.find(item) != view.end())
                {
                    view.erase(item);
                    p_srv->current_conns_cnt_ --;
                }
            }

            p_srv->pending_to_remove_.clear(); // do really empty
            assert(p_srv->front_conns_.size() == p_srv->current_conns_cnt_);

            BOOST_LOG_T(info) << " Connection still alive: " << p_srv->current_conns_cnt_;

            continue; // skip show bellow if just wakend up!
        }

        //睡眠了30s，进行检查


        cout << "<<<<<" << to_simple_string(second_clock::universal_time()) << ">>>>>" <<endl;
        p_srv->show_conns_info(false);
        cout << "====================" << endl;
        p_backend_srv->show_conns_info(false);
        cout << "====================" << endl;
    }
    return;
}



} // END NAMESPACE
