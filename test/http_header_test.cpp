#include <boost/test/unit_test.hpp>
 
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

BOOST_AUTO_TEST_SUITE(http_header_test)

BOOST_AUTO_TEST_CASE(demo)
{
    boost::shared_ptr<std::string> ptr = boost::make_shared<std::string>("TAO");
    std::string str = "TAO";

    BOOST_CHECK_EQUAL(*ptr, str);
}

BOOST_AUTO_TEST_CASE(demo2)
{
    boost::shared_ptr<std::string> ptr = boost::make_shared<std::string>("TAO");
    std::string str = "TAO";

    BOOST_CHECK_EQUAL(*ptr, "tao");
}

BOOST_AUTO_TEST_SUITE_END()
