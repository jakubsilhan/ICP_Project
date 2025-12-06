#pragma once

#include <atomic>
#include <tuple>

#include "utils/NonCopyable.hpp"
#include "concurrency/SyncedDeque.hpp"

#define POOL_N_PREALLOCATE 3
#define POOL_N_MAX 5

template<typename T, typename... Args>
    requires std::constructible_from<T, Args...>
class Pool : NonCopyable {
    protected:
        typedef std::unique_ptr<T> T_ptr;
        UniquePtrSyncedDeque<T> freeElements;
        std::atomic<std::size_t> nAllocated{0};
        std::size_t nMax;
        std::tuple<Args...> args;

        void preallocate(const std::size_t n) {
            for (std::size_t i = 0; i < n; i++) {
                freeElements.pushBack(create());
            }
            nAllocated += n;
        }

        T_ptr create() {
            return std::make_unique<T>(std::make_from_tuple<T>(args));
        }
    public:
        void init(const std::tuple<Args...> args, const std::size_t nPreallocate = POOL_N_PREALLOCATE, const std::size_t nMax = POOL_N_MAX) {
            this->args = args;
            this->nMax = nMax;
            preallocate(nPreallocate);
        }

        void init(Args... args) {
            init(std::tuple(args...));
        }

        T_ptr acquire() {
            if (!freeElements.empty()) {
                return freeElements.popFront();
            }

            if (nAllocated < nMax) {
                nAllocated++;
                return create();
            }

            freeElements.wait();
            return freeElements.popFront();
        }

        void release(T_ptr&& element) {
            freeElements.pushBack(std::move(element));
        }
};