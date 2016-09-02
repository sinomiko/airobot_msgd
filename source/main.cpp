#include "general.hpp"

#include "http_server.hpp"
#include "co_worker.hpp"
#include <backend_server.hpp>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

namespace airobot {

void manage_thread(const objects* daemons);
void boost_log_init(const string filename);

}

// global variable
airobot::objects all_daemons = {nullptr, nullptr, nullptr};

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
        BOOST_LOG_T(info) << "Backend Server Runing At:" << ip_addr << ":" << back_port;

        all_daemons.http_server_ = new airobot::http_server(&all_daemons, 
                                                            ip_addr, srv_port, doc_root, concurr_num);
        all_daemons.backend_server_ = new airobot::backend_server(&all_daemons, 
                                                            ip_addr, back_port);
        all_daemons.co_worker_ = new airobot::co_worker(&all_daemons);

        threads.create_thread(
            []{
            cerr << "Main Front ThreadID: " <<boost::this_thread::get_id()<<endl;
            all_daemons.http_server_->run();
        });

        threads.create_thread(
            []{
            cerr << "Backend ThreadID: " <<boost::this_thread::get_id()<<endl;
            all_daemons.backend_server_->run();
        });

        threads.create_thread(
            []{
            cerr<< "CO WORKER ThreadID: " << boost::this_thread::get_id() << endl;
            all_daemons.co_worker_->run();
        });

        threads.create_thread(boost::bind(airobot::manage_thread, &all_daemons));

        threads.join_all();
    }
    catch (std::exception& e)
    {
        BOOST_LOG_T(fatal) << "exception: " << e.what() << endl;;
    }


    return 0;
}
