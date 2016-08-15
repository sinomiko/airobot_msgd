#include <iostream>
#include <string>
 
using std::endl;
using std::string;

#include <http_server.hpp>

int main(int argc, char* argv[])
{
    const string ip_addr = "0.0.0.0";
    unsigned short srv_port = 5598;
    const string doc_root = "./";

    try
    {
        std::cout << "Server Runing At:" << ip_addr << ":" << srv_port << endl;
        std::cout << "DocumentRoot:" << doc_root << endl;
        airobot::http_server srv(ip_addr, srv_port, doc_root);   
        srv.run();
    } 
    catch (std::exception& e)
    {
        std::cerr << "exception: " << e.what() << endl;;
    }

    return 0;
}
