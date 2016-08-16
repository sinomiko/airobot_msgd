#ifndef _HTTP_PARSER_H_
#define _HTTP_PARSER_H_

#include "general.hpp"
#include "http_proto.hpp"
#include <boost/core/noncopyable.hpp>

namespace airobot {

class http_parser: private boost::noncopyable {

public:
    http_parser() = default;

    bool parse_request(const char* ptr);
    std::string request_option(const std::string option_name);
    bool nicely_add_option(const std::string& option_name, const std::string& option_val);

private:
    std::map<std::string, std::string> header_opts_;
};

}

#endif
