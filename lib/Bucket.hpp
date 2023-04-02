#pragma once

#include <vector>
#include <memory_resource>

class Bucket final {
public:

    const std::size_t BlockSize;
    const std::size_t AmountOfBlocks;

    Bucket(const Bucket& other)
            : BlockSize(other.BlockSize),
              AmountOfBlocks(other.AmountOfBlocks),
              free_blocks_counter_(AmountOfBlocks),
              memory_resource_(other.memory_resource_),
              bytes_(static_cast<std::byte*>(memory_resource_->allocate(BlockSize * AmountOfBlocks))),
              in_use_(static_cast<std::byte*>(memory_resource_->allocate(AmountOfBlocks))) {
        set_blocks_free(0, AmountOfBlocks);
    }

    Bucket(Bucket&& other) noexcept
            : BlockSize(other.BlockSize),
              AmountOfBlocks(other.AmountOfBlocks),
              free_blocks_counter_(other.free_blocks_counter_),
              memory_resource_(other.memory_resource_),
              bytes_(other.bytes_),
              in_use_(other.in_use_) {
        other.bytes_ = other.in_use_ = nullptr;
    }

    explicit Bucket(std::size_t block_size, std::size_t amount_of_blocks, std::pmr::memory_resource* memory_resource);


    ~Bucket();

    [[nodiscard]] void* allocate(std::size_t n) noexcept;

    void deallocate(const void* ptr, std::size_t n) noexcept;

    [[nodiscard]] bool contains(const void* ptr) const noexcept;

    [[nodiscard]] constexpr std::size_t GetBlockSize() const noexcept;

    [[nodiscard]] constexpr std::size_t GetAmountOfBlocks() const noexcept;

private:
    [[nodiscard]] std::size_t find_contiguous_blocks(std::size_t n) const noexcept;

    void set_blocks_in_use(std::size_t index, std::size_t n) noexcept;

    void set_blocks_free(std::size_t index, std::size_t n) noexcept;

    std::size_t free_blocks_counter_;
    std::size_t first_free_block_{0};
    std::pmr::memory_resource* memory_resource_;
    std::byte* bytes_;
    std::byte* in_use_;
};


Bucket::Bucket(std::size_t block_size, std::size_t amount_of_blocks, std::pmr::memory_resource* memory_resource)
        : BlockSize(block_size),
          AmountOfBlocks(amount_of_blocks),
          free_blocks_counter_(AmountOfBlocks),
          memory_resource_(memory_resource),
          bytes_(static_cast<std::byte*>(memory_resource->allocate(BlockSize * AmountOfBlocks))),
          in_use_(static_cast<std::byte*>(memory_resource->allocate(AmountOfBlocks))) {
    set_blocks_free(0, AmountOfBlocks);
}


Bucket::~Bucket() {
    memory_resource_->deallocate(bytes_, BlockSize * AmountOfBlocks);
    memory_resource_->deallocate(in_use_, AmountOfBlocks);
}


void* Bucket::allocate(const std::size_t n) noexcept {
    const auto blocks_needed = 1 + ((n - 1) / BlockSize);

    const auto index = find_contiguous_blocks(blocks_needed);
    if (index == AmountOfBlocks) {
        return nullptr;
    }

    free_blocks_counter_ -= blocks_needed;
    set_blocks_in_use(index, blocks_needed);
    return bytes_ + (index * BlockSize);
}

void Bucket::deallocate(const void* ptr, const std::size_t n) noexcept {
    const auto blocks_to_clear = 1 + ((n - 1) / BlockSize);
    const auto index = static_cast<std::size_t>(static_cast<const std::byte*>(ptr) - bytes_) / BlockSize;
    free_blocks_counter_ += blocks_to_clear;
    set_blocks_free(index, blocks_to_clear);
}

bool Bucket::contains(const void* ptr) const noexcept {
    return static_cast<const std::byte*>(ptr) >= bytes_ && static_cast<const std::byte*>(ptr) < bytes_ + AmountOfBlocks;
}

std::size_t Bucket::find_contiguous_blocks(const std::size_t n) const noexcept {
    if (n > free_blocks_counter_) {
        return AmountOfBlocks;
    }
    std::size_t i = first_free_block_;
    std::size_t counter = 0;
    for (; i < first_free_block_ + n; ++i) {
        counter += (static_cast<unsigned char>((in_use_[i / 8] >> (8 - i % 8 - 1))) & 1);
    }

    while (i < AmountOfBlocks && counter != 0) {
        const auto last_bit = i - n;
        counter = counter +
                  (static_cast<unsigned char>((in_use_[i / 8] >> (8 - i % 8 - 1))) & 1) -
                  (static_cast<unsigned char>((in_use_[last_bit / 8] >> (8 - last_bit % 8 - 1))) & 1);
        ++i;
    }
    if (counter != 0) {
        return AmountOfBlocks;
    }
    return i - n;
}

void Bucket::set_blocks_in_use(const std::size_t index, const std::size_t n) noexcept {
    for (std::size_t i = index; i < index + n; ++i) {
        in_use_[i / 8] = static_cast<std::byte>(static_cast<unsigned char>(in_use_[i / 8]) | (1 << (8 - i % 8 - 1)));
    }
    first_free_block_ = (first_free_block_ >= index && first_free_block_ < index + n ? first_free_block_ = index + n : first_free_block_);
    first_free_block_ = (first_free_block_ == AmountOfBlocks ? 0 : first_free_block_);
}


void Bucket::set_blocks_free(const std::size_t index, const std::size_t n) noexcept {
    for (std::size_t i = index; i < index + n; ++i) {
        in_use_[i / 8] = static_cast<std::byte>(static_cast<unsigned char>(in_use_[i / 8]) &
                                                (0xFF - (1 << (8 - i % 8 - 1))));
    }
    first_free_block_ = std::min(first_free_block_, index);
}

constexpr std::size_t Bucket::GetBlockSize() const noexcept {
    return BlockSize;
}

constexpr std::size_t Bucket::GetAmountOfBlocks() const noexcept {
    return AmountOfBlocks;
}

