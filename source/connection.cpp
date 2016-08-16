#include "connection.hpp"
#include "reply.hpp"
#include "http_server.hpp"

#include "json11.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;
using namespace boost::gregorian;

namespace airobot {

connection::connection(boost::shared_ptr<ip::tcp::socket> p_sock,
                       http_server& server):
    touch_time_(second_clock::local_time()), 
    p_sock_(p_sock),
    parser_(),
    server_(server)
{
    p_buffer_ = boost::make_shared<std::vector<char> >(16*1024, 0);
    p_write_  = boost::make_shared<std::vector<char> >(16*1024, 0); 
}

void connection::start()
{
    do_read();
}

void connection::stop()
{
    server_.set_session_id(shared_from_this(), (int64_t)-1);
    p_sock_->close();
}

void connection::do_read()
{
    p_sock_->async_read_some(buffer(*p_buffer_), 
                             boost::bind(&connection::read_handler, 
                                  shared_from_this(), 
                                  boost::asio::placeholders::error, 
                                  boost::asio::placeholders::bytes_transferred));
}

void connection::do_write()
{

    p_sock_->async_write_some(buffer(*p_write_), 
                         boost::bind(&connection::write_handler,
                                  shared_from_this(), 
                                  boost::asio::placeholders::error, 
                                  boost::asio::placeholders::bytes_transferred));
}

void connection::read_handler(const boost::system::error_code& ec, size_t bytes_transferred)
{
    if (!ec && bytes_transferred) 
    {
        cout << &(*p_buffer_)[0] << endl;

        // read more
        //string ret = reply::reply_generate(string(&(*p_buffer_)[0]), http_proto::status::ok); 
        //string ret = reply::reply_generate("<html><body><p>Hello World!</p></body></html>");

        if( parser_.parse_request(p_buffer_->data()) )
        {
            string body = parser_.request_option(http_proto::header_options::request_body);
            if (body != "")
            {
                string json_err;
                auto json_parsed = json11::Json::parse(body, json_err);
                uint64_t session_id = (uint64_t)json_parsed["session_id"].uint64_value();

                if (session_id == 0) 
                {
                    cout << "SESSSION == 0" << endl;
                    memcpy(p_write_->data(), 
                           reply::fixed_reply_ok.c_str(), 
                           reply::fixed_reply_ok.size()+1 );

                    goto ok_return;
                }
                if (server_.request_session_id(shared_from_this()) == 0) 
                {
                    server_.set_session_id(shared_from_this(), session_id);
                    cout << server_.request_session_id(shared_from_this()) << endl;
                    //assert (shared_from_this() == server_.request_connection(session_id));
                }

                if (json_parsed["msg_type"].uint64_value() == 3 ||
                    json_parsed["msg_type"].uint64_value() == 4) 
                {
                    // TODO: 转发到后台

                    memcpy(p_write_->data(), 
                           reply::fixed_reply_ok.c_str(), 
                           reply::fixed_reply_ok.size()+1 );

                    goto ok_return;
                }
                else if (json_parsed["msg_type"].uint64_value() == 1)
                {
                    // TODO: hold connection

                    goto ok_no_return;
                }
            }
        }
    }
    else if (ec != boost::asio::error::operation_aborted)
    {
        cerr << "READ ERROR FOUND!" << endl;
        shared_from_this()->stop();
        return;
    }

    memcpy(p_write_->data(), 
           reply::fixed_reply_error.c_str(), 
           reply::fixed_reply_error.size()+1);

ok_return:
    do_write();

ok_no_return:
    do_read();

    return;
}

void connection::write_handler(const boost::system::error_code& ec, size_t bytes_transferred)
{
    if (!ec && bytes_transferred) 
    {
        cout << "WRITE OK!" << endl;

        //不会主动调读的
        //do_write();
    }
    else if (ec != boost::asio::error::operation_aborted)
    {
        cerr << "WRITE ERROR FOUND!" << endl;
        shared_from_this()->stop();
    }
}


}
