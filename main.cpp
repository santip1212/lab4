#include <iostream>
#include <map>
#include <functional>
#include "PoolAllocator.h"
#include "MyForwardList.h"

int factorial(int n) {
    if (n <= 1) return 1;
    int result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

int main() {
    // 1) создание экземпляра std::map<int, int>
    std::map<int, int> map1;

    // 2) заполнение 10 элементами (ключ 0-9, значение - факториал)
    for (int i = 0; i < 10; ++i) { map1[i] = factorial(i); }

    // 3) создание экземпляра std::map с новым аллокатором, ограниченным 10 элементами
    using MapPoolAlloc = PoolAllocator<std::pair<const int, int>, 10>;
    std::map<int, int, std::less<int>, MapPoolAlloc> map2;

    // 4) заполнение 10 элементами
    for (int i = 0; i < 10; ++i) { map2[i] = factorial(i); }

    // 5) вывод на экран всех значений (ключ и значение разделены пробелом)
    for (const auto& pair : map2) { std::cout << pair.first << " " << pair.second << std::endl; }

    // 6) создание экземпляра своего контейнера для хранения значений типа int
    MyForwardList<int> my_list1;

    // 7) заполнение 10 элементами от 0 до 9
    for (int i = 0; i < 10; ++i) { my_list1.push_back(i); }

    // 8) создание экземпляра своего контейнера с новым аллокатором, ограниченным 10 элементами
    using ListPoolAlloc = PoolAllocator<int, 10>;
    MyForwardList<int, ListPoolAlloc> my_list2;

    // 9) заполнение 10 элементами от 0 до 9
    for (int i = 0; i < 10; ++i) { my_list2.push_back(i); }

    // 10) вывод на экран всех значений, хранящихся в контейнере
    for (auto it = my_list2.begin(); it != my_list2.end(); ++it) { std::cout << *it << " "; }
    std::cout << std::endl;
    return 0;
}