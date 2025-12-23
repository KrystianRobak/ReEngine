#pragma once


#include "CoreExport.h"
#include "Logger.h"

#include <vector>
#include <thread>
#include <functional>
#include <future>
#include <queue>
#include <condition_variable>
#include <atomic>

enum class JobType {
	General,    // Standard logic
	Background  // Asset loading, File I/O (Lower priority, separate threads)
};

class CORE_API ThreadPool {
public:
	ThreadPool() {}
	~ThreadPool();

	void Init(size_t threads = std::thread::hardware_concurrency());

	template<class F, class... Args>
	auto submit(JobType type, F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
		using return_type = std::invoke_result_t<F, Args...>;
		auto task = std::make_shared<std::packaged_task<return_type()>>(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
		);
		std::future<return_type> res = task->get_future();

		// Wrap task to be void() compliant
		std::function<void()> wrapper = [task]() { (*task)(); };

		if (type == JobType::Background) {
			pushToIO(wrapper);
		}
		else {
			pushToCompute(wrapper);
		}
		return res;
	}


	void shutdown();
	void Dispatch(uint32_t elementCount, uint32_t batchSize, std::function<void(uint32_t, uint32_t)> job);

private:
	void pushToCompute(std::function<void()>& task);
	void pushToIO(std::function<void()>& task);

	// Two separate pools to prevent IO from starving Physics/ECS
	std::vector<std::thread> computeThreads;
	std::vector<std::thread> ioThreads;

	// Queues
	std::queue<std::function<void()>> computeQueue;
	std::queue<std::function<void()>> ioQueue;

	// Synchronization
	std::mutex computeMutex;
	std::condition_variable computeCv;

	std::mutex ioMutex;
	std::condition_variable ioCv;

	std::atomic<bool> stop{ false };
};