#pragma once

#include "BucketConfigs.hpp"

template<std::size_t id>
struct bucket_descriptor {
    using type = std::tuple<>;
};

template<>
struct bucket_descriptor<0> {
    using type = std::tuple<bucket_cfg<24, 10'000'000>, bucket_cfg<32, 100'000>, bucket_cfg<1024, 10'000>>;
};

template<std::size_t id>
using bucket_descriptor_t = typename bucket_descriptor<id>::type;

template<std::size_t id>
constexpr bool is_bucket_descriptor_defined() {
    return std::tuple_size_v<bucket_descriptor_t<id>> != 0;
}

template<std::size_t id, std::size_t index>
constexpr std::size_t GetBlockSize() {
    return std::tuple_element_t<index, bucket_descriptor_t<id>>::GetBlockSize();
};

template<std::size_t id, std::size_t index>
constexpr std::size_t GetAmountOfBlocks() {
    return std::tuple_element_t<index, bucket_descriptor_t<id>>::GetAmountOfBlocks();
};

template<std::size_t id>
constexpr std::size_t GetBucketDescriptorSize() {
    return std::tuple_size_v<bucket_descriptor_t<id>>;
}

template<std::size_t id>
constexpr auto GeneratePool(std::pmr::memory_resource* memory_resource) {
    const auto nested_generate_pool = [=]<std::size_t... indexes>(std::index_sequence<indexes...>)
            -> std::array<Bucket, GetBucketDescriptorSize<id>()> {
        return {
                Bucket(std::tuple_element_t<indexes, bucket_descriptor_t<id>>::GetBlockSize(),
                       std::tuple_element_t<indexes, bucket_descriptor_t<id>>::GetAmountOfBlocks(),
                       memory_resource)...
        };
    };
    return nested_generate_pool(std::make_index_sequence<std::tuple_size_v<bucket_descriptor_t<id>>>());
}
