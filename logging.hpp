_Pragma("once")

#include <sys/time.h>

#include <string>
#include <iostream>

#include <boost/log/sinks.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/core/null_deleter.hpp>


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
        void formatter(const boost::log::record_view &view, boost::log::formatting_ostream &os)
        {
            struct timeval tv;
            gettimeofday(&tv, nullptr);

            struct tm *tm = localtime(&(tv.tv_sec));

            os << (boost::format("[%d-%02d-%02d %02d:%02d:%02d.%03d] ") % (1900 + tm->tm_year) % tm->tm_mon % tm->tm_mday % tm->tm_hour % tm->tm_min % tm->tm_sec % (tv.tv_usec)).str();
            os << "[" << boost::log::aux::this_thread::get_id() << "] ";
            os << "[" << view[boost::log::trivial::severity] << "] \t";
            boost::log::value_ref<std::string> fullpath = boost::log::extract<std::string>("File", view);
            os << boost::filesystem::path(fullpath.get()).filename().string() << " ";
            os << boost::log::extract<std::string>("Function", view) << ":";
            os << boost::log::extract<int>("Line", view) << "\t";

            os << view[boost::log::expressions::smessage];
        }

        void init(boost::log::trivial::severity_level aware_level = info, const std::string &log_file_path = "", bool auto_flush = false)
        {
            using text_sink = boost::log::sinks::asynchronous_sink<boost::log::sinks::text_ostream_backend>;
            using multifile_sink = boost::log::sinks::asynchronous_sink<boost::log::sinks::text_multifile_backend>;

            if (log_file_path.empty())
            {
                boost::shared_ptr<text_sink> log_stdout = boost::make_shared<text_sink>();
                log_stdout->locked_backend()->add_stream(
                    boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter()));
                
                // TODO:
                // log_stdout->locked_backend()->auto_flush(auto_flush);
                log_stdout->set_formatter(&formatter);
                boost::log::core::get()->add_sink(log_stdout);
            }
            else
            {
                boost::shared_ptr<multifile_sink> log_files = boost::make_shared<multifile_sink>();

                log_files->locked_backend()->set_file_name_composer(boost::log::sinks::file::as_file_name_composer(
                    boost::log::expressions::stream << log_file_path));
                log_files->set_formatter(&formatter);

                boost::log::core::get()->add_sink(log_files);
            }

            boost::log::core::get()->set_filter(
                boost::log::trivial::severity >= aware_level);
        }

        BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(lg, boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level>)
    }
}

#define LOGGER(sv) BOOST_LOG_SEV(logging::lg::get(), sv)          \
                       << boost::log::add_value("Line", __LINE__) \
                       << boost::log::add_value("File", __FILE__) \
                       << boost::log::add_value("Function", __func__)
