// http://ideone.com/I2nFCT
#include <vector>
#include <iostream>
#include <algorithm>

template<class T>
void printAll(const T& v) {
    for(const auto& e : v) {
        std::cout << e << " ";
    }
    std::cout << "\n";
}

int main() {
    std::vector<int> v { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    printAll(v);

    v.erase(
        std::remove_if(
            std::begin(v), std::end(v)
            , [](const decltype(v)::value_type& e) {
                return (e % 2) == 0;
            }
        ), std::end(v)
    );
    printAll(v);
}
