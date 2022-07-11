#include <iostream>
#include <variant>
#include <string>
#include <vector>
using namespace std;

template <typename... Types>
struct MyStruct {
    template <typename Type>
    MyStruct(Type val) : value(val) { }

    std::variant<Types...> value;

    void Print() const {
        ([&] {
            if (get_if<Types>(&value))
                cout << get<Types>(value) << endl;
        }(), ...);
    }
};

int main() {
    vector<MyStruct<int, double, string>> vals { 666, 3.14, "sososos" };

    for (const auto &val : vals)
        val.Print();
}