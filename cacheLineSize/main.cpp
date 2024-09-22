#include <iostream>
#include <chrono>
#include <pthread.h>

#define PAGE_PADDING 4096
#define OPERATIONS 1000000000
#define TRASH_SIZE 100000

char array[TRASH_SIZE + ARRAY_SIZE + TRASH_SIZE];

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

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    auto getTimeSinceStart = [&start]() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now()-start).count();
    };

    pthread_attr_t attr;
    cpu_set_t cpus;
    pthread_attr_init(&attr);

    size_t p = 0;
    while ((uint64_t(array + p)) % PAGE_PADDING != 0) {
        p++;
    }
    size_t q = p + ARRAY_SIZE - 1;
    
    pthread_t phread1;
    pthread_t phread2;

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

    std::cout << tm2 - tm1 << std::endl;
    return 0;
}
