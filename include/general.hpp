#ifndef _GENERAL_H_
#define _GENERAL_H_

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <string>
using std::string;

#include <cstdint>
using std::int64_t;
using std::uint64_t;


#include <boost/asio.hpp>
using namespace boost::asio;

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp> 

#include <boost/bind.hpp>

namespace airobot {

typedef boost::shared_ptr<ip::tcp::socket> socket_ptr;
class connection;
typedef boost::shared_ptr<connection> connection_ptr;

}



#endif // _GENERAL_H_
