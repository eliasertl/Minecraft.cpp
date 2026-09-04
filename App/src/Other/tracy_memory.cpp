#ifdef MC_DEBUG
#include <cstdlib>
#include <new>
#include <cstdio>
#include <mutex>
#include <tracy/Tracy.hpp>

std::mutex memoryLock;

void* operator new(std::size_t count) {
    std::lock_guard lock(memoryLock);
    auto ptr = malloc(count);
    TracyAlloc(ptr, count);
    return ptr;
}

void operator delete(void* ptr) noexcept {
    std::lock_guard lock(memoryLock);
    TracyFree(ptr);
    free(ptr);
}

void operator delete[](void* ptr, std::size_t) noexcept {
    std::lock_guard lock(memoryLock);
    TracyFree(ptr);
    free(ptr);
}
#endif