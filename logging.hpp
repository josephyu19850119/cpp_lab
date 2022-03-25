_Pragma("once")

#include <sys/time.h>

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

    namespace logging
{
    using boost::log::trivial::debug;
    using boost::log::trivial::error;
    using boost::log::trivial::fatal;
    using boost::log::trivial::info;
    using boost::log::trivial::trace;
    using boost::log::trivial::warning;

    namespace
    {
        void init(boost::log::trivial::severity_level aware_level = info, const std::string &log_file_path = "")
        {
            typedef boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend> text_sink;
            boost::shared_ptr<text_sink> sink = boost::make_shared<text_sink>();

            if (log_file_path.empty())
            {
                sink->locked_backend()->add_stream(
                    boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter()));
            }
            else
            {
                sink->locked_backend()->add_stream(
                    boost::make_shared<std::ofstream>(log_file_path));
            }

            sink->set_formatter([](boost::log::record_view const &rec, boost::log::formatting_ostream &strm)
                                {
    struct timeval tv;
    gettimeofday(&tv, nullptr);

    struct tm *tm = localtime(&(tv.tv_sec));

    strm << (boost::format("[%d-%02d-%02d %02d:%02d:%02d.%03d] ") % (1900 + tm->tm_year) % tm->tm_mon % tm->tm_mday % tm->tm_hour % tm->tm_min % tm->tm_sec % (tv.tv_usec)).str();
    strm << "[" << boost::log::aux::this_thread::get_id() << "] ";
    strm << "[" << rec[boost::log::trivial::severity] << "] \t";
    boost::log::value_ref<std::string> fullpath = boost::log::extract<std::string>("File", rec);
    strm << boost::filesystem::path(fullpath.get()).filename().string() << " ";
    strm << boost::log::extract<std::string>("Function", rec) << ":";
    strm << boost::log::extract<int>("Line", rec) << "\t";

    strm << rec[boost::log::expressions::smessage]; });

            boost::log::core::get()->add_sink(sink);

            boost::log::core::get()->set_filter(
                boost::log::trivial::severity >= aware_level);

            boost::log::add_common_attributes();
        }

        boost::log::sources::severity_logger<boost::log::trivial::severity_level> lg;
    }
}

#define LOGGER(sv) BOOST_LOG_SEV(logging::lg, sv)                 \
                       << boost::log::add_value("Line", __LINE__) \
                       << boost::log::add_value("File", __FILE__) \
                       << boost::log::add_value("Function", __func__)
