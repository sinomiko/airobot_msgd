#ifndef _REPLY_H_
#define _REPLY_H_

#include "general.hpp"

namespace airobot {

struct header
{
  std::string name;
  std::string value;
};


static const char header_name_value_separator[] = { ':', ' ' };
static const char header_crlf[] = { '\r', '\n'};

static string header_name_value_separator_str = ": ";
static string header_crlf_str = "\r\n";

class reply
{
public:
    reply() = delete;

    static std::string fixed_reply_error;
    static std::string fixed_reply_ok;

    static string reply_generate(const string& content)
    {
        std::vector<header> headers(3);

        // reply fixed header
        headers[0].name = "Content-Length";
        headers[0].value = std::to_string((long long unsigned)content.size());
        headers[1].name = "Content-Type";
        headers[1].value = "text/html";
        headers[2].name = "Connection";
        headers[2].value = "keep-alive";

        string str = "";
        for (int i=0; i< headers.size(); ++i)
        {
            str += headers[i].name;
            str += header_name_value_separator_str;
            str += headers[i].value;
            str += header_crlf_str;
        }
        
        str += header_crlf_str;
        str += content;

        return str;
    }
};

}

#endif
