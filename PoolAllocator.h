#pragma once
#include <cstddef>
#include <memory>
#include <vector>
#include <new>
#include <stdexcept>
#include <limits>
#include <iostream>

template <typename T, std::size_t BlockSize = 10> // выделение 10 ячеек памяти 
class PoolAllocator {
public:
    using value_type = T; // одно из требования к сущ аллок

    template <typename U>
    struct rebind {
        using other = PoolAllocator<U, BlockSize>;
    };

private:
    // Внутренний класс, который хранит сам пул памяти
    struct Pool {
        struct FreeNode {
            FreeNode* next;
        };
        std::size_t total_bytes_allocated = 0;
        std::size_t total_bytes_deallocated = 0;
        
        FreeNode* freeList = nullptr;
        std::vector<void*> allocatedBlocks; // блоки пула

        Pool() = default;
        
        // Деструктор освобождает всю память 
        ~Pool() {
            std::cout << "Allocated memory: " << total_bytes_allocated << " bytes" << std::endl;
            std::cout << "Deallocated memory: " << total_bytes_deallocated << " bytes" << std::endl; 
            for (void* block : allocatedBlocks) {
                ::operator delete(block);
            }
        }

        // Выделение памяти
        void* allocate(std::size_t n) {

            if(n == 0) return nullptr; // проверка на 0

            if (n > std::numeric_limits<std::size_t>::max() / sizeof(T)) { // проверка на переполнение
            throw std::bad_alloc(); 
        }

            if (n != 1) {
                std::size_t bytes = n * sizeof(T);
                void* p = ::operator new(bytes);
                // Запоминаем выделенную память, чтобы удалить при деструкторе
                allocatedBlocks.push_back(p); 
                total_bytes_allocated += bytes; 
                return p;
            }

            if (freeList == nullptr) {
                // Если свободных ячеек нет, выделяем новый блок
                std::size_t allocSize = BlockSize * sizeof(T);
                // Выделяем память через глобальный оператор new
                void* newBlock = ::operator new(allocSize);
                allocatedBlocks.push_back(newBlock);
                total_bytes_allocated += allocSize; // Учитываем память под блок

                // Разбиваем блок на ячейки и добавляем их в freeList
                char* ptr = static_cast<char*>(newBlock);
                for (std::size_t i = 0; i < BlockSize; ++i) {
                    FreeNode* node = reinterpret_cast<FreeNode*>(ptr + i * sizeof(T));
                    node->next = freeList;
                    freeList = node;
                }
            }

            // Забираем одну ячейку из freeList
            FreeNode* node = freeList;
            freeList = freeList->next;
            return node;
        }

        // Поэлементное освобождение памяти
        void deallocate(void* p, std::size_t n) {
            if (n != 1) {
                total_bytes_deallocated += n * sizeof(T);
                ::operator delete(p);
                return;
            }
            total_bytes_deallocated += sizeof(T);
            // Возвращаем ячейку в freeList
            FreeNode* node = static_cast<FreeNode*>(p);
            node->next = freeList;
            freeList = node;
        }
    };

    std::shared_ptr<Pool> pool_;

public:
    PoolAllocator() : pool_(std::make_shared<Pool>()) {}
    
    template <typename U>
    PoolAllocator(const PoolAllocator<U, BlockSize>& other) noexcept : pool_(other.pool_) {}

    // Метод выделения памяти 
    T* allocate(std::size_t n) {
        return static_cast<T*>(pool_->allocate(n));
    }

    // Метод освобождения памяти
    void deallocate(T* p, std::size_t n) {
        pool_->deallocate(p, n);
    }

    // Операторы сравнения нужны для контейнеров STL 
    template <typename U>
    bool operator==(const PoolAllocator<U, BlockSize>& other) const noexcept {
        return pool_ == other.pool_;
    }

    template <typename U>
    bool operator!=(const PoolAllocator<U, BlockSize>& other) const noexcept {
        return !(*this == other);
    }
};