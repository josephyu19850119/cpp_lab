#include <ostream>
#include <fstream>
#include <chrono>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/format.hpp>

void my_formatter(boost::log::record_view const &rec, boost::log::formatting_ostream &strm)
{

    struct timeval tv;
    gettimeofday(&tv, nullptr);

    struct tm *tm = localtime(&(tv.tv_sec));

    strm << boost::log::extract<int>("Line", rec) << ": ";
    strm << boost::log::extract<std::string>("Function", rec) << ": ";
    strm << (boost::format("%02d:%02d:%02d.%03d") % tm->tm_hour % tm->tm_min % tm->tm_sec % (tv.tv_usec / 1000)).str() << ": ";
    boost::log::value_ref<std::string> fullpath = boost::log::extract<std::string>("File", rec);
    strm << boost::filesystem::path(fullpath.get()).filename().string() << ": ";

    // The same for the severity level.
    // The simplified syntax is possible if attribute keywords are used.
    strm << "<" << rec[boost::log::trivial::severity] << "> ";

    // Finally, put the record message to the stream
    strm << rec[boost::log::expressions::smessage];
}

void init()
{
    typedef boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend> text_sink;
    boost::shared_ptr<text_sink> sink = boost::make_shared<text_sink>();


    // sink->locked_backend()->add_stream(
    //     boost::make_shared<std::ofstream>("sample.log"));
    sink->locked_backend()->add_stream(
        boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter()));

        
    sink->set_formatter(&my_formatter);

    boost::log::core::get()->add_sink(sink);

    boost::log::core::get()->set_filter(
        boost::log::trivial::severity >= boost::log::trivial::info);
        
    boost::log::add_common_attributes();
}

boost::log::sources::severity_logger<boost::log::trivial::severity_level> lg;
#define MY_GLOBAL_LOGGER(sv) BOOST_LOG_SEV(lg, sv)                            \
                                       << boost::log::add_value("Line", __LINE__)     \
                                       << boost::log::add_value("File", __FILE__)     \
                                       << boost::log::add_value("Function", __func__) 

int main(int, char *[])
{
    init();


    MY_GLOBAL_LOGGER(boost::log::trivial::debug) << "Keep";
    MY_GLOBAL_LOGGER(boost::log::trivial::info) << "It";
    MY_GLOBAL_LOGGER(boost::log::trivial::warning) << "Simple";
    MY_GLOBAL_LOGGER(boost::log::trivial::error) << "Stupid";

    return 0;
}