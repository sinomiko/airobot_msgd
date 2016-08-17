#include <iostream>
#include <string>
 
using std::endl;
using std::string;

#include <http_server.hpp>
#include <backend_server.hpp>

#include <boost/thread.hpp>

int main(int argc, char* argv[])
{
    const string ip_addr = "0.0.0.0";
    unsigned short srv_port = 5598;
    unsigned short back_port = 5590;
    const string doc_root = "./";

    boost::thread_group threads;

    try
    {
        std::cout << "Server Runing At:" << ip_addr << ":" << srv_port << endl;
        std::cout << "DocumentRoot:" << doc_root << endl;
        //airobot::http_server srv(ip_addr, srv_port, doc_root); 
        boost::shared_ptr<airobot::http_server> p_srv = 
            boost::make_shared<airobot::http_server>(ip_addr, srv_port, doc_root); 

        std::cout << "Backend Server Runing At:" << ip_addr << ":" << back_port << endl;
        //airobot::backend_server backend_srv(ip_addr, back_port);   
        boost::shared_ptr<airobot::backend_server> p_backend_srv = 
            boost::make_shared<airobot::backend_server>(ip_addr, back_port); 

        p_srv->set_backend(p_backend_srv);
        p_backend_srv->set_http(p_srv);

        threads.create_thread(
            [&p_srv]{ 
            cerr<<boost::this_thread::get_id()<<endl; 
            p_srv->run(); 
        });

        threads.create_thread(
            [&p_backend_srv]{ 
            cerr<<boost::this_thread::get_id()<<endl; 
            p_backend_srv->run(); 
        });

        threads.join_all();
    }
    catch (std::exception& e)
    {
        std::cerr << "exception: " << e.what() << endl;;
    }


    return 0;
}
