#include "../lib/PoolAllocator.hpp"
#include <vector>

int main() {
    PoolAllocator<int, 0> allocator;
    for (int i = 0; i < 10000; ++i) {
        std::vector<int, PoolAllocator<int, 0>> vec(allocator);
        for (int j = 0; j < 100'000; ++j) {
            vec.push_back(123);
        }
    }

    return 0;
}
