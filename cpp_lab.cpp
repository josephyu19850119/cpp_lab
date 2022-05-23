#include <iostream>
#include <thread>
#include <string>
using namespace std;

void func(int n, string &s)
{
    cout << "Print int value: " << n << endl;
    cout << "Print original string reference: " << s << endl;

    // Update string content by reference
    s = "updated content";
}

template <typename func, typename... _Args>
void caller(func f, int priority, _Args &&...__args)
{
    thread([&]()
           {
        cout << "set thread priority: " << priority << endl;
        f(__args...); })
        .join();
}

int main(int argc, char **argv)
{
    string s = "mutable string content";

    caller(func, 1, 666, s);

    cout << "Print updated string content by new thread: " << s << endl;

    return 0;
}