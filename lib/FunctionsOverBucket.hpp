#pragma once

#include "Bucket.hpp"

template<std::size_t...>
struct is_empty_variadic : std::true_type {
};

template<std::size_t v, std::size_t... values>
struct is_empty_variadic<v, values...> : std::false_type {
};


std::size_t variadic_max() {
    return 0;
}

std::size_t variadic_max(std::size_t a) {
    return a;
}

std::size_t variadic_max(std::size_t a, std::size_t b) {
    return (a > b ? a : b);
}

template<typename... Args>
std::size_t variadic_max(std::size_t x, Args... y) {
    std::size_t k = Max(y...);
    return (x > k ? x : k);
}