#include "connection.hpp"

namespace airobot {

connection::connection(boost::shared_ptr<ip::tcp::socket> p_sock):
    touch_time_(second_clock::local_time()), 
    p_sock_(p_sock)
{

}

void connection::start()
{

}

void connection::stop()
{

}

}
