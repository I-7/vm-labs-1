#include <iostream>
#include <chrono>
#include <limits>
#include <vector>
#include <algorithm>

#define PAGE_PADDING 4096

#define OPERATIONS 100000000

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    auto getTimeSinceStart = [&start](){
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count();
    };

    char* bufferPrepare = (char*)malloc(4 * CACHE_SIZE);
    if (!bufferPrepare) {
        throw new std::bad_alloc();
    }
    for (size_t i = 0; i < 4 * CACHE_SIZE; i++) {
        bufferPrepare[i] = 0;
    }

    // buffer is assumed to be of size 3 * CACHE_SIZE
    char* buffer = bufferPrepare;
    while (((uint64_t)buffer) % PAGE_PADDING != 0) {
        buffer++;
    }

    uint64_t* trash = (uint64_t*)(buffer + 2 * CACHE_SIZE);

    auto testCacheAssoc = [&buffer, &trash, &getTimeSinceStart](size_t distinctAddresses){
        // Fill the cache by some trash values
        for (size_t i = 0; i < CACHE_SIZE; i += sizeof(uint64_t)) {
            trash[i / sizeof(uint64_t)] = std::numeric_limits<uint64_t>::max();
        }

        // Prepare next access indices
        for (size_t i = 0; i < CACHE_SIZE; i += CACHE_SIZE / distinctAddresses) {
            *(char**)(buffer + i) = buffer + i + CACHE_SIZE / distinctAddresses;
        }
        *(char**)(buffer + CACHE_SIZE - CACHE_SIZE / distinctAddresses) = buffer;

        // Run measuring
        auto tm1 = getTimeSinceStart();
        char* addr = buffer;
        for (int i = 0; i < OPERATIONS; i++) {
            addr = *(char**)(addr);
        }
        auto tm2 = getTimeSinceStart();

        #ifndef QUIET
        std::cout << "Time for test with " << distinctAddresses << " distinct addresses: " << tm2 - tm1 << std::endl;
        #endif
        return tm2 - tm1;
    };

    auto res1 = testCacheAssoc(1);
    size_t cacheAssoc = 0;
    for (size_t n = 2; n <= CACHE_SIZE / CACHE_LINE_SIZE; n <<= 1) {
        auto res = testCacheAssoc(n);
        if (res1 * 1.7 < res && cacheAssoc == 0) {
            cacheAssoc = n >> 1;
            break;
        }
    }
    #ifndef QUIET
    std::cout << "Measured cache associativity: ";
    #endif
    std::cout << cacheAssoc;
    #ifndef QUIET
    std::cout << std::endl;
    #endif

    free(bufferPrepare);
    return 0;
}
