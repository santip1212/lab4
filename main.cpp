#include <iostream>
#include <map>
#include <functional>
#include <memory_resource>
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
    std::pmr::unsynchronized_pool_resource pool;
    std::pmr::map<int, int> map_pmr{&pool};
    // 2) заполнение 10 элементами (ключ 0-9, значение - факториал)
    for (int i = 0; i < 10; ++i) { map_pmr[i] = factorial(i); }
    

    std::pmr::monotonic_buffer_resource mono_res;
    std::pmr::map<int, int> map_pmr2{&mono_res};
    for (int i = 0; i < 10; ++i) { 
        map_pmr2[i] = factorial(i); 
    }

    // 5) вывод на экран всех значений (ключ и значение разделены пробелом)
    for (const auto& pair : map_pmr) { std::cout << pair.first << " " << pair.second << std::endl; }

    using MapPoolAlloc = PoolAllocator<std::pair<const int, int>, 10>;
    std::map<int, int, std::less<int>, MapPoolAlloc> map_custom;
    
    for (int i = 0; i < 10; ++i) { 
        map_custom[i] = factorial(i); 
    }
    
    for (const auto& pair : map_custom) { 
        std::cout << pair.first << " " << pair.second << std::endl; 
    }
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