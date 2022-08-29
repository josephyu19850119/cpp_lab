#include <iostream>
using namespace std;

#include <signal.h>

#include <fastdds_statistics_backend/exception/Exception.hpp>
#include <fastdds_statistics_backend/listener/DomainListener.hpp>
#include <fastdds_statistics_backend/StatisticsBackend.hpp>
#include <fastdds_statistics_backend/types/EntityId.hpp>
#include <fastdds_statistics_backend/types/types.hpp>

using namespace eprosima::statistics_backend;

class Listener : public eprosima::statistics_backend::PhysicalListener
{
public:
    // //! Callback when a new Topic is discovered
    // void on_topic_discovery(
    //     EntityId domain_id,
    //     EntityId topic_id,
    //     const DomainListener::Status &status) override;

    //! Callback when a new Participant is discovered
    void on_participant_discovery(
        EntityId domainId,
        EntityId participantId,
        const DomainListener::Status &status) override
    {
        cout << "Nortify from participant: " << participantId << " in domain " << domainId << endl;

        cout << "status.current_count_change: " << status.current_count_change << endl;

        Info participantInfo = StatisticsBackend::get_info(participantId);
        cout << "Participant Info:" << endl;
        cout << participantInfo.dump(4) << endl;
    }

    //! Callback when a new DataReader is discovered
    void on_datareader_discovery(
        EntityId domainId,
        EntityId datareaderId,
        const DomainListener::Status &status) override
    {
        cout << "Nortify from reader: " << datareaderId << " in domain " << domainId << endl;

        cout << "status.current_count_change: " << status.current_count_change << endl;

        Info participantInfo = StatisticsBackend::get_info(datareaderId);
        cout << "Reader Info:" << endl;
        cout << participantInfo.dump(4) << endl;
    }

    //! Callback when a new DataWriter is discovered
    void on_datawriter_discovery(
        EntityId domainId,
        EntityId datawriterId,
        const DomainListener::Status &status) override
    {
        cout << "Nortify from writer: " << datawriterId << " in domain " << domainId << endl;

        cout << "status.current_count_change: " << status.current_count_change << endl;

        Info writerInfo = StatisticsBackend::get_info(datawriterId);
        cout << "Writer Info:" << endl;
        cout << writerInfo.dump(4) << endl;
    }
};

DomainId domainId = 0;

int main(int argc, char **argv)
{
    if (argc > 1)
    {
        domainId = atoi(argv[1]);
    }

    EntityId monitorId = StatisticsBackend::init_monitor(domainId);
    if (!monitorId.is_valid())
    {
        std::cout << "Error creating monitor" << std::endl;
        return 1;
    }

    Listener listner;
    StatisticsBackend::set_physical_listener(&listner);

    signal(SIGINT, [](int)
    {
        StatisticsBackend::stop_monitor(domainId);
        cout << "StatisticsBackend::stop_monitor" << endl;
    });

    pause();

    return 0;
}