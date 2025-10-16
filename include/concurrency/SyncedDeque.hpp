#include <deque>
#include <mutex>
#include <condition_variable>
#include <optional>

template<typename T>
class SyncedDeque {
protected:
	std::mutex mux;
	std::deque<T> de_queue;
	std::condition_variable cv_sleep;
	std::mutex mux_sleep;

public:
	SyncedDeque() = default;
	SyncedDeque(const SyncedDeque<T>&) = delete;
	~SyncedDeque() {
		clear();
	}
	
	const T& front() {
		std::scoped_lock lock(mux);
		return de_queue.front();
	}

	const T& back() {
		std::scoped_lock lock(mux);
		return de_queue.back();
	}

	T popFront() {
		std::scoped_lock lock(mux);
		auto t = std::move(de_queue.front());
		de_queue.pop_front();
		return t;
	}

	std::optional<T> tryPopFront() {
		std::scoped_lock lock(mux);
		if (de_queue.empty())
			return std::nullopt;

		auto t = std::move(de_queue.front());
		de_queue.pop_front();
		return t;
	}


	T popBack() {
		std::scoped_lock lock(mux);
		auto t = std::move(de_queue.back());
		de_queue.pop_back();
		return t;
	}

	void pushBack(const T& item) {
		std::scoped_lock lock(mux);
		de_queue.emplace_back(std::move(item));

		std::unique_lock<std::mutex> ul(mux_sleep);
		cv_sleep.notify_one();
	}

	void pushFront(const T& item) {
		std::scoped_lock lock(mux);
		de_queue.emplace_front(std::move(item));

		std::unique_lock<std::mutex> ul(mux_sleep);
		cv_sleep.notify_one();
	}
	bool empty() {
		std::scoped_lock lock(mux);
		return de_queue.empty();
	}

	size_t count() {
		std::scoped_lock lock(mux);
		return de_queue.size();
	}

	void clear() {
		std::scoped_lock lock(mux);
		de_queue.clear();
	}

	void wait() {
		while (empty()) {
			std::unique_lock<std::mutex> ul(mux_sleep);
			cv_sleep.wait(ul);
		}
	}

};