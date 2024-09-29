#include <iostream>
#include <chrono>
#include <limits>
#include <vector>
#include <algorithm>

#define PAGE_PADDING 4096

#define MAX_CACHE_SIZE_TO_TEST (1 << 28)
#define MEASURED_CACHE_LINE_SIZE 64

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    auto getTimeSinceStart = [&start](){
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now()-start).count();
    };

    char* bufferPrepare = (char*)malloc(4 * MAX_CACHE_SIZE_TO_TEST);
    if (!bufferPrepare) {
        throw new std::bad_alloc();
    }
    for (size_t i = 0; i < 4 * MAX_CACHE_SIZE_TO_TEST; i++) {
        bufferPrepare[i] = 0;
    }

    // buffer is assumed to be of size 3 * MAX_CACHE_SIZE_TO_TEST
    char* buffer = bufferPrepare;
    while (((uint64_t)buffer) % PAGE_PADDING != 0) {
        buffer++;
    }

    // The first MAX_CACHE_SIZE_TO_TEST bytes of byffer are used for testing
    // The last MAX_CACHE_SIZE_TO_TEST bytes of byffer are used to fill the cache before testing
    uint64_t* trash = (uint64_t*)(buffer + 2 * MAX_CACHE_SIZE_TO_TEST);
    // The middle MAX_CACHE_SIZE_TO_TEST bytes of byffer are used to avoid trash appearing during testing

    auto testCacheSize = [&buffer, &trash, &getTimeSinceStart](size_t n, bool fl){
        // Fill the cache by some trash values
        for (size_t i = 0; i < MAX_CACHE_SIZE_TO_TEST; i += sizeof(uint64_t)) {
            trash[i / sizeof(uint64_t)] = std::numeric_limits<uint64_t>::max();
        }

        // Try to put the whole buffer into the cache
        for (size_t i = 0; i < n; i++) {
            buffer[i] = i & 255;
        }

        uint64_t iter_cnt = MAX_CACHE_SIZE_TO_TEST / n * MEASURED_CACHE_LINE_SIZE;
        char val = 0;
        auto tm1 = getTimeSinceStart();
        for (int iter = 0; iter < iter_cnt; iter++) {
            for (size_t i = 0; i < n; i += MEASURED_CACHE_LINE_SIZE) {
                if (fl) val ^= buffer[i];
            }
        }
        auto tm2 = getTimeSinceStart();
        return tm2 - tm1;
    };

    for (size_t n = MEASURED_CACHE_LINE_SIZE; n <= MAX_CACHE_SIZE_TO_TEST; n <<= 1) {
        std::cout << "Running testCacheSize with n = " << n << std::endl;
        auto result = testCacheSize(n, true) - testCacheSize(n, false);
        std::cout << "Result = " << result << std::endl;
    }

    free(bufferPrepare);
    return 0;
}
