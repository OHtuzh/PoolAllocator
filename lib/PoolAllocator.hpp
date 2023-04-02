#pragma once

#include "SecretPoolAllocator.hpp"

#include <memory>

template<typename T, std::size_t id = 0>
class PoolAllocator : public SecretPoolAllocator<id> {
public:
    static_assert(is_bucket_descriptor_defined<id>(), "There's no such configuration!");

    using value_type = T;

    template<class U>
    struct rebind {
        using other = PoolAllocator<U, id>;
    };

    explicit PoolAllocator(std::pmr::memory_resource* memory_resource = std::pmr::get_default_resource())
            : SecretPoolAllocator<id>(memory_resource),
              memory_resource_(memory_resource) {}

    PoolAllocator(const PoolAllocator& other)
            : SecretPoolAllocator<id>(static_cast<const SecretPoolAllocator<id>&>(other)),
              memory_resource_(other.memory_resource_) {}

    PoolAllocator(PoolAllocator&& other) noexcept
            : SecretPoolAllocator<id>(std::move(static_cast<const SecretPoolAllocator<id>&>(other))),
              memory_resource_(other.memory_resource_) {}

    template<typename U>
    explicit PoolAllocator(const PoolAllocator<U, id>& other)
            :  SecretPoolAllocator<id>(static_cast<const SecretPoolAllocator<id>&>(other)),
               memory_resource_(other.GetMemoryResource()) {}


    template<typename U>
    explicit PoolAllocator(PoolAllocator<U, id>&& other)
            : SecretPoolAllocator<id>(std::move(static_cast<SecretPoolAllocator<id>&>(other))),
              memory_resource_(other.GetMemoryResource()) {}


    PoolAllocator& operator=(const PoolAllocator& other);

    PoolAllocator& operator=(PoolAllocator&& other) noexcept;

    template<typename U>
    PoolAllocator& operator=(const PoolAllocator<U, id>& other);

    template<typename U>
    PoolAllocator& operator=(PoolAllocator<U, id>&& other) noexcept;

    [[nodiscard]] T* allocate(std::size_t n);

    void deallocate(T* p, std::size_t n);

    bool operator==(const PoolAllocator& other);

    bool operator!=(const PoolAllocator& other);

    [[nodiscard]] std::pmr::memory_resource* GetMemoryResource() const noexcept {
        return memory_resource_;
    }

private:
    std::pmr::memory_resource* memory_resource_;
};

template<typename T, std::size_t id>
PoolAllocator<T, id>& PoolAllocator<T, id>::operator=(const PoolAllocator& other) {
    if (this == &other) {
        return *this;
    }
    memory_resource_ = other.memory_resource_;
    *this = static_cast<SecretPoolAllocator<id>&>(other);
    return *this;
}

template<typename T, std::size_t id>
PoolAllocator<T, id>& PoolAllocator<T, id>::operator=(PoolAllocator&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    memory_resource_ = other.memory_resource_;
    *this = std::move(static_cast<SecretPoolAllocator<id>&>(other));
    return *this;
}

template<typename T, std::size_t id>
template<typename U>
PoolAllocator<T, id>& PoolAllocator<T, id>::operator=(const PoolAllocator<U, id>& other) {
    memory_resource_ = other.memory_resource_;
    *this = static_cast<const SecretPoolAllocator<id>&>(other);
    return *this;
}

template<typename T, std::size_t id>
template<typename U>
PoolAllocator<T, id>& PoolAllocator<T, id>::operator=(PoolAllocator<U, id>&& other) noexcept {
    memory_resource_ = other.memory_resource_;
    *this = std::move(static_cast<const SecretPoolAllocator<id>&>(other));
    return *this;
}

template<typename T, std::size_t id>
T* PoolAllocator<T, id>::allocate(std::size_t n) {
    std::array<Info, GetBucketDescriptorSize<id>()> find_best;
    std::size_t index = 0;
    for (auto& bucket: *(this->pool_)) {
        find_best[index].index = index;
        if (bucket.GetBlockSize() >= n) {
            find_best[index].waste = bucket.GetBlockSize() - n;
            find_best[index].amount_of_blocks = 1;
        } else {
            const auto blocks_needed = 1 + ((n - 1) / bucket.GetBlockSize());
            find_best[index].waste = blocks_needed * bucket.GetBlockSize() - n;
            find_best[index].amount_of_blocks = blocks_needed;
        }
        ++index;
    }
    std::sort(find_best.begin(), find_best.end());

    for (const auto& info_element: find_best) {
        auto ptr = (*(this->pool_))[info_element.index].allocate(n * sizeof(T));
        if (ptr != nullptr) {
            return static_cast<T*>(ptr);
        }
    }

    throw std::bad_alloc{};
}

template<typename T, std::size_t id>
void PoolAllocator<T, id>::deallocate(T* p, std::size_t n) {
    for (auto& bucket: *(this->pool_)) {
        if (bucket.contains(p)) {
            bucket.deallocate(p, n * sizeof(T));
            return;
        }
    }
}

template<typename T, std::size_t id>
bool PoolAllocator<T, id>::operator==(const PoolAllocator& other) {
    return this->pool_.get() == other.pool_.get();
}

template<typename T, std::size_t id>
bool PoolAllocator<T, id>::operator!=(const PoolAllocator& other) {
    return !this->operator==(other);
}

