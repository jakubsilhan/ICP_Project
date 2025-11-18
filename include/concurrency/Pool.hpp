#pragma once

#include <atomic>

#include "utils/NonCopyable.hpp"
#include "concurrency/SyncedDeque.hpp"

#define POOL_N_PREALLOCATE 3
#define POOL_N_MAX 5

template<typename T>
class Pool : NonCopyable {
    protected:
        typedef std::unique_ptr<T> T_ptr;
        UniquePtrSyncedDeque<T> freeElements;
        std::atomic<std::size_t> nAllocated{0};
        const std::size_t nMax;

        void preallocate(const std::size_t n) {
            for (std::size_t i = 0; i < n; i++) {
                freeElements.pushBack(std::make_unique<T>());
            }
            nAllocated += n;
        }
    public:
        Pool(const std::size_t nPreallocate = POOL_N_PREALLOCATE, const std::size_t nMax = POOL_N_MAX) : nMax(nMax) {
            preallocate(nPreallocate);
        }
        
        T_ptr acquire() {
            if (!freeElements.empty()) {
                return freeElements.popFront();
            }

            if (nAllocated < nMax) {
                nAllocated++;
                return std::make_unique<T>();
            }

            freeElements.wait();
            return freeElements.popFront();
        }

        void release(T_ptr&& element) {
            freeElements.pushBack(std::move(element));
        }
};