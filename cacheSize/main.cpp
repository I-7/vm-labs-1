#include <iostream>
#include <chrono>
#include <limits>
#include <vector>
#include <algorithm>

#define PAGE_PADDING 4096

#define MAX_CACHE_SIZE_TO_TEST (1 << 28)

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    auto getTimeSinceStart = [&start](){
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count();
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

    auto testCacheSize = [&buffer, &trash, &getTimeSinceStart](size_t n){
        // Fill the cache by some trash values
        for (size_t i = 0; i < MAX_CACHE_SIZE_TO_TEST; i += sizeof(uint64_t)) {
            trash[i / sizeof(uint64_t)] = std::numeric_limits<uint64_t>::max();
        }

        // Prepare next access indices
        for (size_t i = 0; i < n; i += CACHE_LINE_SIZE) {
            *(char**)(buffer + i) = buffer + i + CACHE_LINE_SIZE;
        }
        *(char**)(buffer + n - CACHE_LINE_SIZE) = buffer;

        // Run measuring
        auto tm1 = getTimeSinceStart();
        char* addr = buffer;
        for (int i = 0; i < MAX_CACHE_SIZE_TO_TEST; i++) {
            addr = *(char**)(addr);
        }
        auto tm2 = getTimeSinceStart();

        #ifndef QUIET
        std::cout << "Time for test with array of size " << n << ": " << tm2 - tm1 << std::endl;
        #endif
        return tm2 - tm1;
    };

    auto res1 = testCacheSize(CACHE_LINE_SIZE);
    size_t cacheSize = 0;
    for (size_t n = CACHE_LINE_SIZE << 1; n <= MAX_CACHE_SIZE_TO_TEST; n <<= 1) {
        auto res = testCacheSize(n);
        if (res1 * 1.5 < res && cacheSize == 0) {
            cacheSize = n >> 1;
        }
    }
    #ifndef QUIET
    std::cout << "Measured cache size: ";
    #endif
    std::cout << cacheSize;
    #ifndef QUIET
    std::cout << std::endl;
    #endif

    free(bufferPrepare);
    return 0;
}
