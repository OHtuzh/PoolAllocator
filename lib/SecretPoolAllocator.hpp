#include "BucketDescriptors.hpp"
#include "Info.hpp"

#include <memory>

template<std::size_t id>
class SecretPoolAllocator {
public:
    SecretPoolAllocator() = delete;

protected:

    SecretPoolAllocator(const SecretPoolAllocator& other)
            : pool_(other.pool_) {}

    SecretPoolAllocator(SecretPoolAllocator&& other) noexcept
            : pool_(std::move(other.pool_)) {}

    explicit SecretPoolAllocator(std::pmr::memory_resource* memory_resource)
            : pool_(std::make_shared<std::array<Bucket, GetBucketDescriptorSize<id>()>>(GeneratePool<id>(memory_resource))) {}


    std::shared_ptr<std::array<Bucket, GetBucketDescriptorSize<id>()>> pool_;
};
