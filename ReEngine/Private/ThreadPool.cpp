#include "ThreadPool.h"

ThreadPool::~ThreadPool() {
	shutdown();
}

void ThreadPool::Init(size_t threads)
{
	if (threads == 0) threads = 1;
	for (size_t i = 0; i < threads; ++i) {
		workers.emplace_back([this]() {
			while (!this->stop.load()) {
				std::function<void()> task;
				{
					std::unique_lock<std::mutex> lock(this->queueMutex);
					this->condition.wait(lock, [this]() { return this->stop.load() || !this->tasks.empty(); });
					if (this->stop.load() && this->tasks.empty()) return;
					task = std::move(this->tasks.front());
					this->tasks.pop();
				}
				task();
			}
			});
	}
}


void ThreadPool::shutdown() {
	bool expected = false;
	if (!stop.compare_exchange_strong(expected, true)) return; // already stopping
	condition.notify_all();
	for (auto& worker : workers) {
		if (worker.joinable()) worker.join();
	}
}


