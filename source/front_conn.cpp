#include "front_conn.hpp"
#include "reply.hpp"
#include "http_server.hpp"
#include "http_proto.hpp"
#include "json11.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;
using namespace boost::gregorian;

#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>

namespace airobot {

namespace http_opts = http_proto::header_options;
namespace http_stat = http_proto::status;

front_conn::front_conn(boost::shared_ptr<ip::tcp::socket> p_sock,
                       http_server& server):
    strand_(server.io_service_),
    connection(p_sock),
    parser_(),
    server_(server)
{
    // p_buffer_ & p_write_ 
    // already allocated @ connection
}

void front_conn::start()
{
    /**
     * 这里需要注意的是，如果do_read()不是虚函数，而派生类只是简单的覆盖，
     * 那么在accept　handler中调用的new_c->start()会导致这里会调用基类
     * 版本的do_read
     */
    set_stats(conn_working);
    do_read_head();
}

void front_conn::stop()
{
    //server_.set_session_id(shared_from_this(), (int64_t)-1);
    set_stats(conn_pending);
}

// Wrapping the handler with strand.wrap. This will return a new handler, that will dispatch through the strand.
// Posting or dispatching directly through the strand.

void front_conn::do_read_head()
{
    if (get_stats() != conn_working)
    {
        BOOST_LOG_T(error) << "SOCK STATUS: " << get_stats();
        return;
    }

    BOOST_LOG_T(info) << "strand read read_util ... in " << boost::this_thread::get_id();
    async_read_until(*p_sock_, request_,
                             "\r\n\r\n",
                             strand_.wrap(
                                 boost::bind(&front_conn::read_head_handler,
                                     shared_from_this(),
                                     boost::asio::placeholders::error,
                                     boost::asio::placeholders::bytes_transferred)));
    return;
}


void front_conn::do_read_body()
{
    if (get_stats() != conn_working)
    {
        BOOST_LOG_T(error) << "SOCK STATUS: " << get_stats();
        return;
    }

    size_t len = ::atoi(parser_.request_option(http_opts::content_length).c_str());

    BOOST_LOG_T(info) << "strand read async_read exactly... in " << boost::this_thread::get_id();
    async_read(*p_sock_, buffer(p_buffer_->data() + r_size_, len - r_size_),
                    boost::asio::transfer_exactly(len - r_size_), 
                             strand_.wrap(
                                 boost::bind(&front_conn::read_body_handler,
                                     shared_from_this(),
                                  boost::asio::placeholders::error,
                                  boost::asio::placeholders::bytes_transferred)));
    return;
}

void front_conn::do_write()
{
    if (get_stats() != conn_working)
    {
        BOOST_LOG_T(error) << "SOCK STATUS: " << get_stats();
        return;
    }

    assert(w_size_);
    assert(w_pos_ < w_size_);

    BOOST_LOG_T(info) << "strand write async_write exactly... in " << boost::this_thread::get_id();
    async_write(*p_sock_, buffer(p_write_->data() + w_pos_, w_size_ - w_pos_),
                    boost::asio::transfer_exactly(w_size_ - w_pos_),
                              strand_.wrap(
                                boost::bind(&front_conn::write_handler,
                                     shared_from_this(),
                                  boost::asio::placeholders::error,
                                  boost::asio::placeholders::bytes_transferred)));
    return;
}

void front_conn::read_head_handler(const boost::system::error_code& ec, size_t bytes_transferred)
{
    if (!ec && bytes_transferred) 
    {
        std::string head_str (boost::asio::buffers_begin(request_.data()), 
                              boost::asio::buffers_begin(request_.data()) + request_.size());

        request_.consume(bytes_transferred); // skip the head
        if (parser_.parse_request(head_str.c_str()))
        {
            if (! boost::iequals(parser_.request_option(http_opts::request_method), "POST") )
            {
                BOOST_LOG_T(error) << "Invalid request method: " << parser_.request_option(http_opts::request_method);
                fill_for_http(http_proto::content_bad_request, http_proto::status::bad_request);
                goto write_return;
            }

            if ( boost::iequals(parser_.request_option(http_opts::request_uri), "/aa") ||
                 boost::iequals(parser_.request_option(http_opts::request_uri), "/bb") ||
                 boost::iequals(parser_.request_option(http_opts::request_uri), "/cc") )
            {
                size_t len = ::atoi(parser_.request_option(http_opts::content_length).c_str());
                r_size_ = 0;

                // first async_read_until may read more additional data, if so
                // then move additional data possible
                if( request_.size() )
                {
                    r_size_ = request_.size();
                    std::string additional (boost::asio::buffers_begin(request_.data()), 
                              boost::asio::buffers_begin(request_.data()) + r_size_);
                    memcpy(p_buffer_->data(), additional.c_str(), r_size_ );
                    request_.consume(r_size_); // skip the head
                }

                // normally, we will return these 2 cases
                if (r_size_ < len)
                {
                    // need to read more data
                    do_read_body();
                    return;
                }
                else
                {
                    // call the process callback directly
                    boost::system::error_code ec;
                    read_body_handler(ec, r_size_);
                    return;
                }
            }
            else
            {       
                BOOST_LOG_T(error) << "Invalid request uri: " << parser_.request_option(http_opts::request_uri);
                fill_for_http(http_proto::content_not_found, http_proto::status::not_found); 
                goto write_return;
            }

            // default, OK
            goto write_return;
        }
        else
            {
            BOOST_LOG_T(error) << "Parse request error: " << head_str << endl;
            goto error_return;
        }
    }
    else if (ec != boost::asio::error::operation_aborted)
    {
        BOOST_LOG_T(error) << "READ ERROR FOUND!";

        boost::system::error_code ignored_ec;
        p_sock_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
        p_sock_->cancel();

        notify_conn_error();
        return;
    }

error_return:
    fill_for_http(http_proto::content_error, http_proto::status::internal_server_error); 

write_return:
    do_write();

    // if error, we will not read anymore
    // notify_conn_error();

    return;
}


void front_conn::read_body_handler(const boost::system::error_code& ec, size_t bytes_transferred)
{
    if (!ec && bytes_transferred)
    {

        size_t len = ::atoi(parser_.request_option(http_opts::content_length).c_str());
        r_size_ += bytes_transferred;

        if (r_size_ < len)
        {
            // need to read more!
            do_read_body();
            return;
        }

        if ( boost::iequals(parser_.request_option(http_opts::request_uri), "/aa") )
        {
            fill_for_http("aa", http_proto::status::ok);
        }
        else if ( boost::iequals(parser_.request_option(http_opts::request_uri), "/bb") )
        {
            // 假设bb的请求转发到后台

            string body = string(p_buffer_->data(), r_size_);
            body = string(body.c_str());
            string json_err;
            auto json_parsed = json11::Json::parse(body, json_err);

            if (!json_err.empty())
            {
                BOOST_LOG_T(error) << "JSON parse error: " << body;
                fill_for_http("bb_ret", http_proto::status::ok);
                goto write_return;
            }

            uint64_t session_id = (uint64_t)json_parsed["session_id"].uint64_value();
            uint64_t site_id = (uint64_t)json_parsed["site_id"].uint64_value();

            cout << "request info: " << session_id << " " << site_id << endl;
            server_.push_backend(site_id, body.c_str(), body.size()+1);

            // 后台的返回
            fill_for_http("bb_ret", http_proto::status::ok);
        }
        else if ( boost::iequals(parser_.request_option(http_opts::request_uri), "/cc") )
        {
            fill_for_http("cc", http_proto::status::ok);
        }
        else
        {
            BOOST_LOG_T(error) << "Invalid request uri: " << parser_.request_option(http_opts::request_uri);
            fill_for_http(http_proto::content_not_found, http_proto::status::not_found); 
            goto write_return;
        }

        // default, OK
        goto write_return;
    }
    else if (ec != boost::asio::error::operation_aborted)
    {
        BOOST_LOG_T(error) << "READ ERROR FOUND: " << ec;

        boost::system::error_code ignored_ec;
        p_sock_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
        p_sock_->cancel();

        notify_conn_error();
        return;
    }

error_return:
    fill_for_http(http_proto::content_error, http_proto::status::internal_server_error); 

write_return:
    do_write();

    do_read_head();

    return;
}


void front_conn::write_handler(const boost::system::error_code& ec, size_t bytes_transferred)
{
    if (!ec && bytes_transferred) 
    {
        //assert(bytes_transferred == w_size_);
        //w_size_ = 0;

        w_pos_ += bytes_transferred;

        if (w_pos_ < w_size_) 
        {
            BOOST_LOG_T(error) << "ADDITIONAL WRITE: " << w_pos_ << " ~ " << w_size_;
            do_write();
        }
        else
        {
            w_pos_ = w_size_ = 0;
        }
    }
    else if (ec != boost::asio::error::operation_aborted)
    {
        BOOST_LOG_T(error) << "WRITE ERROR FOUND:" << ec;

        boost::system::error_code ignored_ec;
        p_sock_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
        p_sock_->cancel();

        notify_conn_error();
    }
}


void front_conn::notify_conn_error()
{
    {
        boost::lock_guard<boost::mutex> lock(server_.conn_notify_mutex); 
        r_size_ = 0;
        w_size_ = 0;
        set_stats(conn_error);
    }
    server_.conn_notify.notify_one();
}

}
