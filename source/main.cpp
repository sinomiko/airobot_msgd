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
        airobot::http_server srv(ip_addr, srv_port, doc_root); 

        std::cout << "Backend Server Runing At:" << ip_addr << ":" << back_port << endl;
        airobot::backend_server backend_srv(ip_addr, back_port);   

        threads.create_thread(
            [&srv]{ 
            cerr<<boost::this_thread::get_id()<<endl; 
            srv.run(); 
        });

        threads.create_thread(
            [&backend_srv]{ 
            cerr<<boost::this_thread::get_id()<<endl; 
            backend_srv.run(); 
        });

        threads.join_all();
    }
    catch (std::exception& e)
    {
        std::cerr << "exception: " << e.what() << endl;;
    }


    return 0;
}
