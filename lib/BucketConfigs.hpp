#pragma once

#include "Bucket.hpp"

template<std::size_t BlockSize, std::size_t AmountOfBlocks>
struct bucket_cfg {
    static inline constexpr std::size_t GetBlockSize() noexcept {
        return BlockSize;
    }

    static inline constexpr std::size_t GetAmountOfBlocks() noexcept {
        return AmountOfBlocks;
    }
};
