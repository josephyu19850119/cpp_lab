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

        template <typename Type>
        message(const std::string &_name, Type _value)
            : name(_name.c_str()), value(_value)
        {
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
        void write(const message &msg)
        {
            auto func = [this, msg]()
            {
                std::pair<message_buffer *, std::size_t> ptr = domain_shared_memory.find<message_buffer>(channel_name.c_str());
                if (ptr.first != nullptr)
                {
                    (ptr.first)->push_back(msg);
                }
            };
            domain_shared_memory.atomic_func(func);
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

        reader(int domain_id, const std::string &_channel_name)
            : channel_name(_channel_name),
              domain_shared_memory(boost::interprocess::managed_shared_memory{boost::interprocess::open_only, domain_shared_memory_name_formatter(domain_id).c_str()})
        {
        }

    public:
        std::vector<message> read()
        {
            std::vector<message> result;

            auto func = [this, &result]()
            {
                std::pair<const message_buffer *, std::size_t> pipe = domain_shared_memory.find<message_buffer>(channel_name.c_str());
                if (pipe.first != nullptr)
                {
                    for (typename message_buffer::const_iterator iter = (pipe.first)->begin() + offset; iter != (pipe.first)->end(); ++iter)
                    {
                        result.push_back(*iter);
                        ++offset;
                    }
                }
            };

            try
            {
                this->domain_shared_memory.atomic_func(func);
            }
            catch (const std::exception &e)
            {
                 e.what();
            }

            return result;
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
            e.what();
            return std::optional<reader>();
        }
    }
};