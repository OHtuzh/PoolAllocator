#pragma once

#include <cinttypes>

struct Info {
    Info() = default;

    Info(std::size_t index, std::size_t amount_of_blocks, std::size_t waste) noexcept
            : index(index),
              amount_of_blocks(amount_of_blocks),
              waste(waste) {}

    bool operator<(const Info& other) const noexcept {
        return (waste == other.waste) ? amount_of_blocks < other.amount_of_blocks : waste < other.waste;
    }

    std::size_t index{0};
    std::size_t amount_of_blocks{0};
    std::size_t waste{0};

};
