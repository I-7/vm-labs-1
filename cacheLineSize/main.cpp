#include <iostream>
#include <chrono>
#include <pthread.h>

#define PAGE_PADDING 4096
#define MAX_CACHE_LINE_SIZE_TO_TEST 1024

#define OPERATIONS 1000000000
#define TRASH_SIZE 100000

char* array;

void* inc(void* args) {
    size_t index = ((size_t*)args)[0];
    char& elem_p = array[index];
    for (uint64_t i = 0; i < OPERATIONS; i++) {
        elem_p++;
        if (elem_p >= 32)
            elem_p >>= 1;
    }
    return nullptr;
}

auto test(int array_size) {
    auto start = std::chrono::high_resolution_clock::now();

    auto getTimeSinceStart = [&start]() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count();
    };

    array = (char*)malloc(TRASH_SIZE + array_size + TRASH_SIZE);
    size_t p = TRASH_SIZE;
    while ((uint64_t(array + p)) % PAGE_PADDING != 0) {
        p++;
    }
    size_t q = p + array_size - 1;
    
    pthread_t phread1;
    pthread_t phread2;

    pthread_attr_t attr;
    cpu_set_t cpus;
    pthread_attr_init(&attr);

    auto tm1 = getTimeSinceStart();
    CPU_ZERO(&cpus);
    CPU_SET(0, &cpus);
    pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
    pthread_create(&phread1, &attr, inc, &p);

    CPU_ZERO(&cpus);
    CPU_SET(1, &cpus);
    pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
    pthread_create(&phread2, &attr, inc, &q);

    pthread_join(phread1, nullptr);
    pthread_join(phread2, nullptr);
    auto tm2 = getTimeSinceStart();

    #ifndef QUIET
    std::cout << "Time for test with array of size " << array_size << ": " << tm2 - tm1 << std::endl;
    #endif
    return tm2 - tm1;
}

int main() {
    auto res1 = test(1);
    size_t cacheLineSize = 0;
    for (int i = 1; (1 << i) <= MAX_CACHE_LINE_SIZE_TO_TEST; i++) {
        size_t array_size = (1 << i) + 1;
        auto res = test(array_size);
        if (res * 2 < res1 && cacheLineSize == 0) {
            cacheLineSize = (1 << i);
        }
    }
    #ifndef QUIET
    std::cout << "Measured cache line size: ";
    #endif
    std::cout << cacheLineSize;
    #ifndef QUIET
    std::cout << std::endl;
    #endif
}
