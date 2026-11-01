#pragma once
#include <cstddef>
#include <memory>
#include <vector>
#include <new>
#include <stdexcept>

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
        
        FreeNode* freeList = nullptr;
        std::vector<void*> allocatedBlocks; // блоки пула

        Pool() = default;
        
        // Деструктор освобождает всю память 
        ~Pool() {
            for (void* block : allocatedBlocks) {
                ::operator delete(block);
            }
        }

        // Выделение памяти
        void* allocate(std::size_t n) {
            if (n != 1) {
                // Если просят больше 1 элемента, выделяем отдельный блок 
               return ::operator new(n * sizeof(T));
            }

            if (freeList == nullptr) {
                // Если свободных ячеек нет, выделяем новый блок
                std::size_t allocSize = BlockSize * sizeof(T);
                // Выделяем память через глобальный оператор new
                void* newBlock = ::operator new(allocSize);
                allocatedBlocks.push_back(newBlock);

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
                ::operator delete(p);
                return;
            }
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