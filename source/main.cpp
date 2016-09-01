#include "general.hpp"

#include "http_server.hpp"
#include "co_worker.hpp"
#include <backend_server.hpp>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

namespace airobot {

void manage_thread(boost::shared_ptr<http_server> p_srv,
                     boost::shared_ptr<backend_server> p_backend_srv);
void boost_log_init(const string filename);

}


int main(int argc, char* argv[])
{
    const string ip_addr = "0.0.0.0";
    unsigned short srv_port = 5598;
    unsigned short back_port = 5590;
    const string doc_root = "./";

    // ignore sigpipe
    assert(::signal(SIGPIPE, SIG_IGN) != SIG_ERR);

    airobot::boost_log_init("airobot_running");

    boost::thread_group threads;

    size_t concurr_num = boost::thread::hardware_concurrency();
    BOOST_LOG_T(info) << "System hardware_concurrency: " << concurr_num;

    try
    {
        BOOST_LOG_T(info) << "Server Runing At:" << ip_addr << ":" << srv_port;
        BOOST_LOG_T(info) << "DocumentRoot:" << doc_root;

        boost::shared_ptr<airobot::http_server> p_srv =
            boost::make_shared<airobot::http_server>(ip_addr, srv_port, doc_root, concurr_num);
        boost::shared_ptr<airobot::co_worker>   p_co_worker =
            boost::make_shared<airobot::co_worker>();

        BOOST_LOG_T(info) << "Backend Server Runing At:" << ip_addr << ":" << back_port;
        //airobot::backend_server backend_srv(ip_addr, back_port);
        boost::shared_ptr<airobot::backend_server> p_backend_srv =
            boost::make_shared<airobot::backend_server>(ip_addr, back_port);

        p_srv->set_backend(p_backend_srv);
        p_backend_srv->set_http(p_srv);

        threads.create_thread(
            [&p_srv]{
            cerr << "Main Front ThreadID: " <<boost::this_thread::get_id()<<endl;
            p_srv->run();
        });

        threads.create_thread(
            [&p_backend_srv]{
            cerr << "Backend ThreadID: " <<boost::this_thread::get_id()<<endl;
            p_backend_srv->run();
        });

        threads.create_thread(
            [&p_co_worker]{
            cerr<< "CO WORKER ThreadID: " << boost::this_thread::get_id() << endl;
            p_co_worker->run();
        });

        threads.create_thread(boost::bind(airobot::manage_thread, p_srv, p_backend_srv));

        threads.join_all();
    }
    catch (std::exception& e)
    {
        BOOST_LOG_T(fatal) << "exception: " << e.what() << endl;;
    }


    return 0;
}
