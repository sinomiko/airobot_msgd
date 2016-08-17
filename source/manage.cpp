#include "general.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;
using namespace boost::gregorian;

#include <boost/thread.hpp>

#include "http_server.hpp"
#include "backend_server.hpp"

/**
 * 包含管理维护线程的相关工作
 */

namespace airobot {



void manage_thread(boost::shared_ptr<http_server> p_srv,
                     boost::shared_ptr<backend_server> p_backend_srv)
{
    for (;;)
    {
        boost::this_thread::sleep(boost::posix_time::seconds(2));
        cout << "<<<<<" << to_simple_string(second_clock::universal_time()) << ">>>>>" <<endl;
        p_srv->show_conns_info(false);
        cout << "====================" << endl;
        p_backend_srv->show_conns_info(false);
        cout << "====================" << endl;
    }
    return;
}



} // END NAMESPACE
