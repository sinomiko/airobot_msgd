#include "general.hpp"
#include "http_parser.hpp"

namespace airobot {
    
    bool http_parser::parse_request(const char* ptr)
    {
        if (!ptr || !strlen(ptr))
            return false;
        
        header_opts.clear();


        return true;
    }

    std::string http_parser::request_option(const std::string option_name)
    {
        auto search = header_opts.find(option_name); 
        if (search != header_opts.end()) 
            return search->second;
        
        return "";
    }

}
