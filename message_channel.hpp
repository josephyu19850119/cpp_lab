_Pragma("once");

#include <variant>
#include <string>
#include <vector>
#include <optional>
#include <iostream>

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/vector.hpp>

namespace
{
    static std::string domain_shared_memory_name_formatter(int domain_id)
    {
        return "unios_domain_" + std::to_string(domain_id) + "_shared_memory";
    }
}

template <typename... Types>
class message_channel
{
public:
    struct message
    {
        boost::interprocess::string name;
        std::variant<Types...> value;
        timeval timestamp;

        template <typename Type>
        message(const std::string &_name, Type _value)
            : name(_name.c_str()), value(_value)
        {
            gettimeofday(&timestamp, nullptr);
        }
    };

    static const unsigned default_shared_memory_size = 1024 * 1024;

private:
    using message_buffer_manager = boost::interprocess::allocator<message, boost::interprocess::managed_shared_memory::segment_manager>;
    using message_buffer = boost::interprocess::vector<message, message_buffer_manager>;

public:
    class writer
    {
        friend message_channel<Types...>;
        std::string channel_name;
        boost::interprocess::managed_shared_memory domain_shared_memory;

        writer(int domain_id, const std::string &_channel_name, unsigned size)
            : channel_name(_channel_name),
              domain_shared_memory(boost::interprocess::managed_shared_memory{boost::interprocess::open_or_create, domain_shared_memory_name_formatter(domain_id).c_str(), size})
        {
            std::pair<const message_buffer *, std::size_t> ptr = domain_shared_memory.find<message_buffer>(channel_name.c_str());
            if (ptr.first == nullptr)
            {
                domain_shared_memory.construct<message_buffer>(channel_name.c_str())(domain_shared_memory.get_segment_manager());
            }
        }

    public:
        bool write(const message &msg)
        {
            bool succeed = true;
            auto func = [this, msg, &succeed]()
            {
                std::pair<message_buffer *, std::size_t> ptr = domain_shared_memory.find<message_buffer>(channel_name.c_str());
                if (ptr.first != nullptr)
                {
                    (ptr.first)->push_back(msg);
                }
                else
                {
                    succeed = false;
                }
            };
            domain_shared_memory.atomic_func(func);

            return succeed;
        }
    };

    static writer open_writer(int domain_id, const std::string &channel_name, unsigned size = default_shared_memory_size)
    {
        return writer(domain_id, channel_name, size);
    }

    class reader
    {
        friend message_channel<Types...>;
        std::string channel_name;
        boost::interprocess::managed_shared_memory domain_shared_memory;
        typename message_buffer::size_type offset = 0;
        std::string err_text;

        reader(int domain_id, const std::string &_channel_name)
            : channel_name(_channel_name),
              domain_shared_memory(boost::interprocess::managed_shared_memory{boost::interprocess::open_only, domain_shared_memory_name_formatter(domain_id).c_str()})
        {
        }

    public:
        bool read(std::vector<message> &msgs)
        {
            bool succeed = true;
            err_text.clear();

            auto func = [this, &succeed, &msgs]()
            {
                std::pair<const message_buffer *, std::size_t> pipe = domain_shared_memory.find<message_buffer>(channel_name.c_str());
                if (pipe.first != nullptr)
                {
                    for (typename message_buffer::const_iterator iter = (pipe.first)->begin() + offset; iter != (pipe.first)->end(); ++iter)
                    {
                        msgs.push_back(*iter);
                        ++offset;
                    }
                }
                else
                {
                    err_text = "Not found message channel: " + channel_name;
                    succeed = false;
                }
            };

            try
            {
                this->domain_shared_memory.atomic_func(func);
            }
            catch (const std::exception &e)
            {
                err_text = e.what();
                return false;
            }

            return succeed;
        }

        std::string get_err_text() const
        {
            return err_text;
        }
    };

    static std::optional<reader> open_reader(int domain_id, const std::string &channel_name)
    {
        try
        {
            return reader(domain_id, channel_name);
        }
        catch (const std::exception &e)
        {
            return std::optional<reader>();
        }
    }

    static bool remove(int domain_id, const std::string &channel_name = "")
    {
        if (channel_name.empty())
        {
            return boost::interprocess::shared_memory_object::remove(domain_shared_memory_name_formatter(domain_id).c_str());
        }
        else
        {
            try
            {
                boost::interprocess::managed_shared_memory domain_shared_memory{boost::interprocess::open_only, domain_shared_memory_name_formatter(domain_id).c_str()};
                return domain_shared_memory.destroy<message_buffer>(channel_name.c_str());
            }
            catch (const boost::interprocess::interprocess_exception &e)
            {
                return false;
            }
        }
    }
};