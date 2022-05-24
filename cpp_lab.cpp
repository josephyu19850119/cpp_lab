#include "unios_task.h"

#include <iostream>
#include <string>
using namespace std;

#include <unistd.h>

int main(int argc, char **argv)
{
    UniOS::Task::Launch([](const string& str){
        UniOS::Task::SubTask([](){
            cout << "in sub Task" << endl;
        });
        cout  << str << endl;
    }, UniOS::Task::TaskPriority::P10, false, "yuyue");

    sleep(5);
    return 0;
}