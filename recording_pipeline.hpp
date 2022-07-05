#include <variant>
#include <string>
#include <vector>

#include <iostream>

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/vector.hpp>

template <typename... _Types>
class recording_pipe
{
public:
    struct record
    {
        boost::interprocess::string name;
        std::variant<_Types...> value;

        template <typename type>
        record(const std::string &_name, type _value)
            : name(_name.c_str()), value(_value)
        {
        }
    };

    static const unsigned default_shared_memory_size = 1024 * 1024;

    using record_vector_manager = boost::interprocess::allocator<record, boost::interprocess::managed_shared_memory::segment_manager>;
    using record_pipe = boost::interprocess::vector<record, record_vector_manager>;

    class writer
    {
        friend recording_pipe<_Types...>;
        typename recording_pipe<_Types...>::recording_pipe<_Types...>::record_pipe *p = nullptr;
        boost::interprocess::managed_shared_memory domain_shared_memory;

        writer(int domain_id, const std::string &pipe_name, unsigned size)
        {
            std::string domain_shared_memory_name = "unios_domain_" + std::to_string(domain_id) + "_shared_memory";

            boost::interprocess::shared_memory_object::remove(domain_shared_memory_name.c_str());
            domain_shared_memory = boost::interprocess::managed_shared_memory{boost::interprocess::open_or_create, domain_shared_memory_name.c_str(), size};
            p = domain_shared_memory.construct<record_pipe>(pipe_name.c_str())(domain_shared_memory.get_segment_manager());
        }

    public:
        void send(const record &rec)
        {
            auto f = [this, rec]()
            {
                this->p->push_back(rec);
            };
            domain_shared_memory.atomic_func(f);
        }
    };

    static writer open_writer(int domain_id, const std::string &pipe_name, unsigned size = recording_pipe<_Types...>::default_shared_memory_size)
    {
        return writer(domain_id, pipe_name, size);
    }

    class reader
    {
        friend recording_pipe<_Types...>;
        std::string pipe_name;
        boost::interprocess::managed_shared_memory domain_shared_memory;
        typename record_pipe::size_type offset = 0;

        reader(int domain_id, const std::string &_pipe_name)
            : pipe_name(_pipe_name.c_str())
        {
            std::string domain_shared_memory_name = "unios_domain_" + std::to_string(0) + "_shared_memory";
            domain_shared_memory = boost::interprocess::managed_shared_memory{boost::interprocess::open_only, domain_shared_memory_name.c_str()};
        }

    public:
        std::vector<record> read()
        {
            std::vector<record> result;

            auto func = [this, &result]()
            {
                std::pair<const record_pipe *, std::size_t> pipe = domain_shared_memory.find<record_pipe>(pipe_name.c_str());
                if (pipe.first != nullptr)
                {
                    for (typename record_pipe::const_iterator iter = (pipe.first)->begin() + offset; iter != (pipe.first)->end(); ++iter)
                    {
                        result.push_back(*iter);
                        ++offset;
                    }
                }
            };
            this->domain_shared_memory.atomic_func(func);

            return result;
        }
    };

    static reader open_reader(int domain_id, const std::string &pipe_name)
    {
        return reader(domain_id, pipe_name);
    }
};