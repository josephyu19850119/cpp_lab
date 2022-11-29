#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <unistd.h>
#include "underlay_message.pb.h"
using namespace std;
using namespace std::chrono;

// Main function:  Reads the entire address book from a file and prints all
//   the information inside.
int main(int argc, char *argv[])
{
    // using SECOND = duration<int>;
    // using SECOND_FLOAT = duration<float>;
    // using MILLI_SECOND = duration<int, milli>;
    // using NANO_SECOND = duration<int, nano>;
    
    // Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against.
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    semidrive::protobuf::common::MessageStream msgs;
    
    msgs.ParseFromIstream(&cin);
    // fstream input(argv[1], ios::in | ios::binary);
    // msgs.ParseFromIstream(&input);


    time_point<std::chrono::high_resolution_clock> start_time_point = high_resolution_clock::now();

    for (int i = 0; i < msgs.messages_size(); ++i)
    {
        cout << msgs.messages()[i].seq() << endl;
        cout << msgs.messages()[i].timestamp() << endl;
        cout << msgs.messages()[i].data_type() << endl;
        cout << msgs.messages()[i].topic_name() << endl;
        cout << msgs.messages()[i].data().size() << endl << endl;

        if (i < msgs.messages_size() - 1)
        {
            this_thread::sleep_until(start_time_point + duration<unsigned long long, nano>(msgs.messages()[i + 1].timestamp() - msgs.messages()[0].timestamp()));
        }
    }

    cerr << "Recorded messsages count: " << msgs.messages_size() << endl;
    duration<float> d = (high_resolution_clock::now() - start_time_point);
    cerr << "duration: " << d.count() << endl;

    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}